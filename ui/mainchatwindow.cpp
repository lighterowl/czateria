#include "mainchatwindow.h"
#include "ui_mainchatwindow.h"

#include <QAction>
#include <QCompleter>
#include <QDateTime>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QImageReader>
#include <QMessageBox>
#include <QPlainTextEdit>
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

QCompleter *createNicknameCompleter(Czateria::UserListModel *userlist,
                                    QObject *parent) {
  auto rv = new QCompleter(userlist, parent);
  rv->setCompletionRole(Qt::DisplayRole);
  rv->setCaseSensitivity(Qt::CaseInsensitive);
  rv->setCompletionMode(QCompleter::InlineCompletion);
  return rv;
}

QString getImageFilter() {
  static QString cached_result;
  if (!cached_result.isNull()) {
    return cached_result;
  }
  auto rv = QObject::tr("Images (");
  auto formats = QImageReader::supportedImageFormats();
  for (auto &&format : formats) {
    rv.append(QString(QLatin1Literal("*.%1"))
                  .arg(QString::fromUtf8(format.constData()).toLower()));
    if (&format != &formats.back()) {
      rv.append(QLatin1Char(' '));
    }
  }
  rv.append(QLatin1Char(')'));
  cached_result = rv;
  return rv;
}
} // namespace

MainChatWindow::MainChatWindow(const Czateria::LoginSession &login,
                               QWidget *parent)
    : QWidget(parent, Qt::Window), ui(new Ui::MainChatWindow),
      mChatSession(new Czateria::ChatSession(login, this)),
      mSortProxy(new QSortFilterProxyModel(this)),
      mNicknameCompleter(
          createNicknameCompleter(mChatSession->userListModel(), this)) {
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
            ui->tabWidget->privateMessageTab(nickname)->appendPlainText(
                QObject::tr("[%1] Image received")
                    .arg(QDateTime::currentDateTime().toString(
                        QLatin1String("HH:mm:ss"))));
          });

  connect(ui->tabWidget, &ChatWindowTabWidget::privateConversationClosed,
          mChatSession,
          &Czateria::ChatSession::notifyPrivateConversationClosed);
  connect(ui->tabWidget, &ChatWindowTabWidget::currentChanged, [=](int tabIdx) {
    // disable completer for private conversations
    ui->lineEdit->setCompleter(tabIdx == 0 ? mNicknameCompleter : nullptr);
  });

  connect(ui->lineEdit, &QLineEdit::returnPressed, this,
          &MainChatWindow::onReturnPressed);
  ui->lineEdit->setCompleter(mNicknameCompleter);

  connect(ui->listView, &QAbstractItemView::doubleClicked, this,
          &MainChatWindow::onUserNameDoubleClicked);
  connect(ui->listView, &UserListView::mouseMiddleClicked, this,
          &MainChatWindow::onUserNameMiddleClicked);

  connect(ui->tabWidget, &QTabWidget::currentChanged,
          [=](auto idx) { ui->sendImageButton->setEnabled(idx != 0); });
  connect(ui->sendImageButton, &QAbstractButton::clicked, [=](auto) {
    auto filename = QFileDialog::getOpenFileName(
        this, tr("Select an image file"), QString(), getImageFilter());
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
    ui->tabWidget->addMessageToCurrent(
        QObject::tr("[%1] Image sent")
            .arg(QDateTime::currentDateTime().toString(
                QLatin1String("HH:mm:ss"))));
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

void MainChatWindow::onUserNameMiddleClicked() {
  auto idx =
      mSortProxy->mapToSource(ui->listView->selectionModel()->currentIndex());
  auto nickname = mChatSession->userListModel()->data(idx).toString();
  ui->lineEdit->insert(nickname);
}
