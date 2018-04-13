#include "mainchatwindow.h"
#include "ui_mainchatwindow.h"

#include <QMessageBox>
#include <QSortFilterProxyModel>

#include <czatlib/chatsession.h>
#include <czatlib/message.h>
#include <czatlib/userlistmodel.h>

namespace {
int getOptimalUserListWidth(QWidget *widget) {
  // max nick length is 16 characters
  static const QString worstCase = QLatin1String("wwwwwwwwwwwwwwww");
  auto font = widget->font();
  font.setBold(true);
  return QFontMetrics(font).size(Qt::TextSingleLine, worstCase).width();
}
} // namespace

MainChatWindow::MainChatWindow(const Czateria::LoginSession &login,
                               QWidget *parent)
    : QWidget(parent, Qt::Window), ui(new Ui::MainChatWindow),
      mChatSession(new Czateria::ChatSession(login, this)),
      mSortProxy(new QSortFilterProxyModel(this)) {
  ui->setupUi(this);
  setWindowTitle(mChatSession->channel());

  auto desiredWidth = getOptimalUserListWidth(ui->listView);
  ui->widget_3->setMaximumSize(QSize(desiredWidth, QWIDGETSIZE_MAX));
  ui->widget_3->setMinimumSize(QSize(desiredWidth, 0));

  mSortProxy->setSourceModel(mChatSession->userListModel());
  mSortProxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
  mSortProxy->setSortLocaleAware(true);
  mSortProxy->setDynamicSortFilter(true);
  connect(ui->lineEdit_2, &QLineEdit::textChanged, mSortProxy,
          &QSortFilterProxyModel::setFilterFixedString);

  ui->listView->setModel(mSortProxy);
  ui->nicknameLabel->setText(mChatSession->nickname());
  setAttribute(Qt::WA_DeleteOnClose);

  connect(mChatSession, &Czateria::ChatSession::roomMessageReceived,
          ui->tabWidget, &ChatWindowTabWidget::displayRoomMessage);
  connect(mChatSession, &Czateria::ChatSession::privateMessageReceived,
          ui->tabWidget, &ChatWindowTabWidget::displayPrivateMessage);
  connect(mChatSession, &Czateria::ChatSession::newPrivateConversation, this,
          &MainChatWindow::onNewPrivateConversation);
  connect(mChatSession, &Czateria::ChatSession::privateConversationStateChanged,
          ui->tabWidget,
          &ChatWindowTabWidget::onPrivateConversationStateChanged);
  connect(mChatSession, &Czateria::ChatSession::nicknameAssigned,
          ui->nicknameLabel, &QLabel::setText);

  connect(ui->tabWidget, &ChatWindowTabWidget::privateConversationClosed,
          mChatSession,
          &Czateria::ChatSession::notifyPrivateConversationClosed);

  connect(ui->lineEdit, &QLineEdit::returnPressed, this,
          &MainChatWindow::onReturnPressed);

  connect(ui->listView, &QAbstractItemView::doubleClicked, this,
          &MainChatWindow::onUserNameDoubleClicked);

  mChatSession->start();
}

MainChatWindow::~MainChatWindow() { delete ui; }

void MainChatWindow::onNewPrivateConversation(const QString &nickname) {
  auto question =
      QObject::tr("%1 wants to talk in private.\nDo you accept?").arg(nickname);
  auto rv = QMessageBox::question(this, QObject::tr("New private conversation"),
                                  question);
  if (rv == QMessageBox::Yes) {
    mChatSession->acceptPrivateConversation(nickname);
    ui->tabWidget->openPrivateMessageTab(nickname);
  } else {
    mChatSession->rejectPrivateConversation(nickname);
  }
}

void MainChatWindow::onReturnPressed() {
  auto text = ui->lineEdit->text();
  auto currentNickname = ui->tabWidget->getCurrentNickname();
  if (currentNickname.isNull()) {
    mChatSession->sendRoomMessage(text);
  } else if (mChatSession->canSendMessage(currentNickname)) {
    mChatSession->sendPrivateMessage(currentNickname, text);
  } else {
    return;
  }
  ui->lineEdit->clear();
  ui->tabWidget->addMessageToCurrent(
      {QDateTime::currentDateTime(), text, mChatSession->nickname()});
}

void MainChatWindow::onUserNameDoubleClicked(const QModelIndex &proxyIdx) {
  auto idx = mSortProxy->mapToSource(proxyIdx);
  auto nickname = mChatSession->userListModel()->data(idx).toString();
  if (nickname != mChatSession->nickname()) {
    ui->tabWidget->openPrivateMessageTab(nickname);
    ui->lineEdit->setFocus(Qt::OtherFocusReason);
  }
}
