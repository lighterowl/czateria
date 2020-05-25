#include "mainchatwindow.h"
#include "appsettings.h"
#include "mainwindow.h"
#include "ui_chatwidget.h"

#include <QAction>
#include <QCompleter>
#include <QDateTime>
#include <QDialogButtonBox>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileDialog>
#include <QImageReader>
#include <QMenuBar>
#include <QMessageBox>
#include <QMimeData>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QStandardPaths>
#include <QToolBar>
#include <QUrl>

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

QString imageDefaultPath(const QString &channel, const QString &nickname) {
  return QString(QLatin1String("%1/czateria_%2_%3_%4.png"))
      .arg(QStandardPaths::writableLocation(QStandardPaths::PicturesLocation))
      .arg(channel)
      .arg(nickname)
      .arg(QDateTime::currentDateTime().toString(
          QLatin1String("yyyyMMddHHmmss")));
}

void showImageDialog(QWidget *parent, const QString &nickname,
                     const QString &channel, const QImage &image) {
  auto imgDialog = new QDialog(parent);
  imgDialog->setAttribute(Qt::WA_DeleteOnClose);
  imgDialog->setModal(false);
  imgDialog->setWindowTitle(MainChatWindow::tr("Image from %1").arg(nickname));
  auto layout = new QVBoxLayout;
  auto label = new QLabel;
  auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Save);
  buttonBox->setCenterButtons(true);
  auto defaultPath = imageDefaultPath(channel, nickname);
  QObject::connect(buttonBox->button(QDialogButtonBox::Save),
                   &QPushButton::clicked, [=](auto) {
                     auto fileName = QFileDialog::getSaveFileName(
                         parent,
                         MainChatWindow::tr("Save image from %1").arg(nickname),
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
  auto rv = MainChatWindow::tr("Images (");
  auto formats = QImageReader::supportedImageFormats();
  for (auto &&format : formats) {
    rv.append(QString(QLatin1String("*.%1"))
                  .arg(QString::fromUtf8(format.constData()).toLower()));
    if (&format != &formats.back()) {
      rv.append(QLatin1Char(' '));
    }
  }
  rv.append(QLatin1Char(')'));
  cached_result = rv;
  return rv;
}

QString explainBlockCause(Czateria::ChatSession::BlockCause why) {
  using c = Czateria::ChatSession::BlockCause;
  switch (why) {
  case c::Nick:
    return MainChatWindow::tr("nick");
  case c::Avatar:
    return MainChatWindow::tr("avatar");
  case c::Behaviour:
    return MainChatWindow::tr("behaviour");
  default:
    return QString();
  }
}

QString getKickBanMsgStr(const QString &blockTypeStr,
                         Czateria::ChatSession::BlockCause why,
                         const QString &adminNick = QString()) {
  auto bannedBy = adminNick.isEmpty()
                      ? QString()
                      : MainChatWindow::tr(" by %1").arg(adminNick);
  auto causeStr = why == Czateria::ChatSession::BlockCause::Unknown
                      ? QString()
                      : MainChatWindow::tr(" for inappropriate %1")
                            .arg(explainBlockCause(why));
  return QString(QLatin1String("You were %1%2%3"))
      .arg(blockTypeStr)
      .arg(bannedBy)
      .arg(causeStr);
}
} // namespace

MainChatWindow::MainChatWindow(QSharedPointer<Czateria::LoginSession> login,
                               Czateria::AvatarHandler &avatars,
                               const Czateria::Room &room,
                               const AppSettings &settings, MainWindow *mainWin)
    : QMainWindow(nullptr), ui(new Ui::ChatWidget),
      mChatSession(new Czateria::ChatSession(login, avatars, room, this)),
      mSortProxy(new QSortFilterProxyModel(this)),
      mNicknameCompleter(
          createNicknameCompleter(mChatSession->userListModel(), this)),
      mAppSettings(settings),
      mAutoAcceptPrivs(
          new QAction(tr("Automatically accept private conversations"), this)),
      mSendImageAction(
          new QAction(QIcon(QLatin1String(":/icons/file-picture-icon.png")),
                      tr("Send an image"), this)),
      mShowChannelListAction(
          new QAction(QIcon(QLatin1String(":/icons/czateria.png")),
                      tr("Show channel list"), this)) {
  QIcon icon;
  icon.addFile(QString::fromUtf8(":/icons/czateria.png"), QSize(),
               QIcon::Normal, QIcon::Off);
  setWindowIcon(icon);
  setAcceptDrops(true);
  auto centralWidget = new QWidget(this);
  ui->setupUi(centralWidget);
  setWindowTitle(mChatSession->channel());
  setCentralWidget(centralWidget);
  statusBar();
  auto toolbar = new QToolBar;
  addToolBar(Qt::TopToolBarArea, toolbar);

  mShowChannelListAction->setToolTip(tr("Show channel list"));
  mShowChannelListAction->setStatusTip(tr("Shows the channel list window"));
  connect(mShowChannelListAction, &QAction::triggered,
          [=](auto) { mainWin->show(); });
  toolbar->addAction(mShowChannelListAction);

  connect(mSendImageAction, &QAction::triggered, [=](auto) {
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
    sendImageToCurrent(image);
  });
  mSendImageAction->setToolTip(tr("Send an image"));
  mSendImageAction->setStatusTip(
      tr("Sends an image to your conversation partner"));
  mSendImageAction->setEnabled(false);
  toolbar->addAction(mSendImageAction);

  auto menu = menuBar()->addMenu(tr("Options"));
  mAutoAcceptPrivs->setStatusTip(
      tr("Accept private conversations without prompting"));
  mAutoAcceptPrivs->setCheckable(true);
  menu->addAction(mAutoAcceptPrivs);

  auto desiredWidth = getOptimalUserListWidth(ui->listView);
  ui->widget_3->setMaximumSize(QSize(desiredWidth, QWIDGETSIZE_MAX));
  ui->widget_3->setMinimumSize(QSize(desiredWidth, 0));

  mSortProxy->setSourceModel(mChatSession->userListModel());
  mSortProxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
  mSortProxy->setSortLocaleAware(true);
  mSortProxy->setDynamicSortFilter(true);
  void (QSortFilterProxyModel::*setFilterFn)(const QString &) =
      &QSortFilterProxyModel::setFilterRegExp;
  connect(ui->lineEdit_2, &QLineEdit::textChanged, mSortProxy, setFilterFn);

  ui->listView->setModel(mSortProxy);
  ui->listView->setUserListModel(mChatSession->userListModel());
  ui->listView->setAvatarHandler(&avatars);

  ui->nicknameLabel->setText(mChatSession->nickname());
  setAttribute(Qt::WA_DeleteOnClose);

  connect(mChatSession, &Czateria::ChatSession::roomMessageReceived,
          ui->tabWidget, &ChatWindowTabWidget::displayRoomMessage);
  connect(mChatSession, &Czateria::ChatSession::privateMessageReceived,
          ui->tabWidget, &ChatWindowTabWidget::displayPrivateMessage);
  connect(mChatSession, &Czateria::ChatSession::privateMessageReceived, this,
          &MainChatWindow::notifyActivity);
  connect(mChatSession, &Czateria::ChatSession::newPrivateConversation, this,
          &MainChatWindow::onNewPrivateConversation);
  connect(mChatSession, &Czateria::ChatSession::privateConversationCancelled,
          this, [=](const QString &nickname) {
            auto msgbox = mPendingPrivRequests.value(nickname, nullptr);
            if (msgbox) {
              msgbox->reject();
              msgbox->deleteLater();
              mPendingPrivRequests.remove(nickname);
            }
          });
  connect(mChatSession, &Czateria::ChatSession::privateConversationStateChanged,
          ui->tabWidget,
          &ChatWindowTabWidget::onPrivateConversationStateChanged);
  connect(mChatSession, &Czateria::ChatSession::nicknameAssigned,
          ui->nicknameLabel, &QLabel::setText);
  connect(mChatSession, &Czateria::ChatSession::imageReceived,
          [=](auto &&nickname, auto &&image) {
            auto textedit = ui->tabWidget->privateMessageTab(nickname);
            if (mAppSettings.savePicturesAutomatically) {
              auto defaultPath =
                  imageDefaultPath(mChatSession->channel(), nickname);
              image.save(defaultPath);
              textedit->appendPlainText(
                  tr("[%1] Image saved as %2")
                      .arg(QDateTime::currentDateTime().toString(
                          QLatin1String("HH:mm:ss")))
                      .arg(defaultPath));
            } else {
              showImageDialog(this, nickname, mChatSession->channel(), image);
              textedit->appendPlainText(
                  tr("[%1] Image received")
                      .arg(QDateTime::currentDateTime().toString(
                          QLatin1String("HH:mm:ss"))));
            }
            notifyActivity();
          });
  connect(mChatSession, &Czateria::ChatSession::sessionExpired, [=]() {
    QMessageBox::information(
        this, tr("Session expired"),
        tr("Your session has expired.\nPlease log back in."));
  });
  connect(mChatSession, &Czateria::ChatSession::sessionError, [=]() {
    QMessageBox::critical(
        this, tr("Communication error"),
        tr("An unknown error has occurred.\nPlease try logging in again, "
           "perhaps with a different nickname."));
  });

  connect(ui->tabWidget, &ChatWindowTabWidget::privateConversationClosed,
          mChatSession,
          &Czateria::ChatSession::notifyPrivateConversationClosed);
  connect(ui->tabWidget, &ChatWindowTabWidget::currentChanged, [=](int tabIdx) {
    // disable completer for private conversations
    ui->lineEdit->setCompleter(tabIdx == 0 ? mNicknameCompleter : nullptr);
    updateWindowTitle();
  });
  connect(mChatSession, &Czateria::ChatSession::banned,
          [=](auto why, auto &&who) {
            QMessageBox::information(this, tr("Banned"),
                                     getKickBanMsgStr(tr("banned"), why, who));
          });
  connect(mChatSession, &Czateria::ChatSession::kicked, [=](auto why) {
    QMessageBox::information(this, tr("Kicked"),
                             getKickBanMsgStr(tr("kicked"), why));
  });

  connect(ui->lineEdit, &QLineEdit::returnPressed, this,
          &MainChatWindow::onReturnPressed);
  ui->lineEdit->setCompleter(mNicknameCompleter);

  connect(ui->listView, &QAbstractItemView::doubleClicked, this,
          &MainChatWindow::onUserNameDoubleClicked);
  connect(ui->listView, &UserListView::mouseMiddleClicked, this,
          &MainChatWindow::onUserNameMiddleClicked);

  connect(ui->tabWidget, &QTabWidget::currentChanged,
          [=](auto idx) { mSendImageAction->setEnabled(idx != 0); });

  mChatSession->start();
}

MainChatWindow::~MainChatWindow() { delete ui; }

void MainChatWindow::onNewPrivateConversation(const QString &nickname) {
  if (mAutoAcceptPrivs->isChecked()) {
    doAcceptPrivateConversation(nickname);
  } else {
    auto question =
        tr("%1 wants to talk in private.\nDo you accept?").arg(nickname);
    auto msgbox =
        new QMessageBox(QMessageBox::Question, tr("New private conversation"),
                        question, QMessageBox::Yes | QMessageBox::No, this);
    mPendingPrivRequests[nickname] = msgbox;
    msgbox->setDefaultButton(QMessageBox::Yes);
    msgbox->button(QMessageBox::Yes)->setShortcut(QKeySequence());
    msgbox->button(QMessageBox::No)->setShortcut(QKeySequence());
    connect(msgbox, &QDialog::finished, [=](int result) {
      switch (result) {
      case QMessageBox::Yes:
        doAcceptPrivateConversation(nickname);
        break;
      case QMessageBox::No:
        mChatSession->rejectPrivateConversation(nickname);
        break;
      default:
        Q_ASSERT(0);
      }
    });
    msgbox->show();
    msgbox->raise();
    msgbox->activateWindow();
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

void MainChatWindow::doAcceptPrivateConversation(const QString &nickname) {
  mChatSession->acceptPrivateConversation(nickname);
  ui->tabWidget->openPrivateMessageTab(nickname);
  ui->lineEdit->setFocus(Qt::OtherFocusReason);
  notifyActivity();
}

void MainChatWindow::notifyActivity() {
  QApplication::alert(this);
  updateWindowTitle();
}

void MainChatWindow::updateWindowTitle() {
  auto unreadPrivs = ui->tabWidget->countUnreadPrivateTabs();
  auto channelName = mChatSession->channel();
  auto windowTitle = channelName;
  if (unreadPrivs) {
    windowTitle =
        QString(QLatin1String("[%1] %2")).arg(unreadPrivs).arg(channelName);
  }
  setWindowTitle(windowTitle);
}

void MainChatWindow::sendImageToCurrent(const QImage &image) {
  mChatSession->sendImage(ui->tabWidget->getCurrentNickname(), image);
  ui->tabWidget->addMessageToCurrent(
      tr("[%1] Image sent")
          .arg(QDateTime::currentDateTime().toString(
              QLatin1String("HH:mm:ss"))));
}

void MainChatWindow::dragEnterEvent(QDragEnterEvent *ev) {
  if (ui->tabWidget->getCurrentNickname().isEmpty()) {
    return;
  }

  const auto mime = ev->mimeData();
  bool has_data = false;
  if (mime->hasImage()) {
    has_data = true;
  } else if (mime->hasUrls()) {
    auto url = mime->urls()[0];
    if (url.isLocalFile()) {
      // TODO caching this value would be nice, but we have no means of
      // modifying the event's QMimeData. currently the QImage needs to be
      // constructed for a second time in dropEvent().
      auto img = QImage(url.toLocalFile());
      has_data = !img.isNull();
    }
  }

  if (has_data) {
    ev->acceptProposedAction();
  }
}

void MainChatWindow::dropEvent(QDropEvent *ev) {
  const auto mime = ev->mimeData();
  QImage img;
  if (mime->hasImage()) {
    img = qvariant_cast<QImage>(ev->mimeData()->imageData());
  } else if (mime->hasUrls()) {
    img = QImage(mime->urls()[0].toLocalFile());
  }
  sendImageToCurrent(img);
  ev->acceptProposedAction();
}
