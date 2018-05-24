#include "mainchatwindow.h"
#include "ui_mainchatwindow.h"

#include <QAction>
#include <QDateTime>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QStandardPaths>

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

void showImageDialog(QWidget *parent, const QString &nickname,
                     const QString &channel, const QImage &image) {
  auto imgDialog = new QDialog(parent);
  imgDialog->setAttribute(Qt::WA_DeleteOnClose);
  imgDialog->setModal(false);
  imgDialog->setWindowTitle(QObject::tr("Image from %1").arg(nickname));
  auto layout = new QVBoxLayout;
  auto label = new QLabel;
  auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Save);
  buttonBox->setCenterButtons(true);
  auto defaultPath = QString(QLatin1String("%1/czateria_%2_%3_%4.png"))
                         .arg(QStandardPaths::writableLocation(
                             QStandardPaths::PicturesLocation))
                         .arg(channel)
                         .arg(nickname)
                         .arg(QDateTime::currentDateTime().toString(
                             QLatin1String("yyyyMMddHHmmss")));
  QObject::connect(buttonBox->button(QDialogButtonBox::Save),
                   &QPushButton::clicked, [=](auto) {
                     auto fileName = QFileDialog::getSaveFileName(
                         parent,
                         QObject::tr("Save image from %1").arg(nickname),
                         defaultPath);
                     if (!fileName.isNull()) {
                       image.save(fileName);
                     }
                   });
  label->setPixmap(QPixmap::fromImage(image));
  layout->addWidget(label);
  layout->addWidget(buttonBox);
  imgDialog->setLayout(layout);
  imgDialog->show();
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
  connect(mChatSession, &Czateria::ChatSession::imageReceived,
          [=](auto &&nickname, auto &&image) {
            showImageDialog(this, nickname, mChatSession->channel(), image);
          });

  connect(ui->tabWidget, &ChatWindowTabWidget::privateConversationClosed,
          mChatSession,
          &Czateria::ChatSession::notifyPrivateConversationClosed);

  connect(ui->lineEdit, &QLineEdit::returnPressed, this,
          &MainChatWindow::onReturnPressed);

  connect(ui->listView, &QAbstractItemView::doubleClicked, this,
          &MainChatWindow::onUserNameDoubleClicked);

  connect(ui->tabWidget, &QTabWidget::currentChanged,
          [=](auto idx) { ui->sendImageButton->setEnabled(idx != 0); });
  connect(ui->sendImageButton, &QAbstractButton::clicked, [=](auto) {
    // TODO specify a filter
    auto filename =
        QFileDialog::getOpenFileName(this, tr("Select an image file"));
    if (filename.isEmpty()) {
      return;
    }
    auto image = QImage(filename);
    if (image.isNull()) {
      QMessageBox::critical(
          this, tr("Not an image"),
          tr("The selected file does not appear to be an image"));
      return;
    }
    mChatSession->sendImage(ui->tabWidget->getCurrentNickname(), image);
  });

  mChatSession->start();
}

MainChatWindow::~MainChatWindow() { delete ui; }

void MainChatWindow::onNewPrivateConversation(const QString &nickname) {
  auto question =
      QObject::tr("%1 wants to talk in private.\nDo you accept?").arg(nickname);
  auto rv = QMessageBox::question(this, QObject::tr("New private conversation"),
                                  question, QMessageBox::Yes | QMessageBox::No,
                                  QMessageBox::Yes);
  if (rv == QMessageBox::Yes) {
    mChatSession->acceptPrivateConversation(nickname);
    ui->tabWidget->openPrivateMessageTab(nickname);
    ui->lineEdit->setFocus(Qt::OtherFocusReason);
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
