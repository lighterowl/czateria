#include "chatwindowtabwidget.h"

#include <QApplication>
#include <QDebug>
#include <QDialogButtonBox>
#include <QLabel>
#include <QPlainTextEdit>
#include <QStackedWidget>
#include <QTabBar>
#include <QVBoxLayout>

#include <czatlib/message.h>

namespace {
QString formatMessage(const Czateria::Message &msg) {
  return QString(QLatin1String("[%1] <%2> %3"))
      .arg(msg.receivedAt().toString(QLatin1String("HH:mm:ss")))
      .arg(msg.nickname())
      .arg(msg.message());
}

QPlainTextEdit *createTextWidget(QWidget *parent) {
  /* TODO : implement actual animated icons, like the website does. this will
   * probably require moving to a normal (non-plain) text edit. */
  auto rv = new QPlainTextEdit(parent);
  rv->setReadOnly(true);
  return rv;
}

const QColor unreadTabColor = QColor(Qt::red);
} // namespace

class ChatWindowTabWidget::PrivateChatTab : public QStackedWidget {
  PrivateChatTab(ChatWindowTabWidget *parent, const QString &nickname)
      : QStackedWidget(parent), mNickname(nickname) {}

  void addTextWidget() {
    Q_ASSERT(!mTextWidget);
    mTextWidget = createTextWidget(this);
    addWidget(mTextWidget);
  }

  void onConversationRejected() { deleteLater(); }

  const QString mNickname;
  QPlainTextEdit *mTextWidget = nullptr;

public:
  friend struct PendingAcceptWidget;
  struct PendingAcceptWidget : public QWidget {
    PendingAcceptWidget(PrivateChatTab *chatTab,
                        ChatWindowTabWidget *chatWindow,
                        const QString &nickname) {
      auto layout = new QVBoxLayout;
      layout->addWidget(
          new QLabel(tr("<b>%1</b> wants to talk in private.\nDo you accept?")
                         .arg(nickname)));
      auto buttons = new QDialogButtonBox(
          QDialogButtonBox::Yes | QDialogButtonBox::No, Qt::Horizontal);
      connect(buttons, &QDialogButtonBox::accepted, [=]() {
        emit chatWindow->privateConversationAccepted(nickname);
        this->deleteLater(); // remove self in order to have StackedWidget
                             // switch to TextEdit
      });
      connect(buttons, &QDialogButtonBox::rejected, [=]() {
        emit chatWindow->privateConversationRejected(nickname);
        chatTab->onConversationRejected();
      });
      layout->addWidget(buttons);
      setLayout(layout);
    }
  };

  const QString &nickname() const { return mNickname; }

  static PrivateChatTab *createAccepted(ChatWindowTabWidget *parent,
                                        const QString &nickname) {
    auto rv = new PrivateChatTab(parent, nickname);
    rv->addTextWidget();
    return rv;
  }

  static PrivateChatTab *create(ChatWindowTabWidget *parent,
                                const QString &nickname) {
    auto rv = new PrivateChatTab(parent, nickname);
    auto pendingWidget = new PendingAcceptWidget(rv, parent, nickname);
    rv->addWidget(pendingWidget);
    rv->addTextWidget();
    return rv;
  }

  void appendPlainText(const QString &text) {
    mTextWidget->appendPlainText(text);
  }
};

ChatWindowTabWidget::ChatWindowTabWidget(QWidget *parent)
    : QTabWidget(parent), mMainChatTab(createTextWidget(this)) {
  setTabBarAutoHide(true);
  setTabsClosable(true);
  addTab(mMainChatTab, tr("Main chat"));
  connect(this, &QTabWidget::tabCloseRequested, this,
          &ChatWindowTabWidget::onTabCloseRequested);
  connect(this, &QTabWidget::currentChanged, this,
          &ChatWindowTabWidget::updateTabActivity);
}

void ChatWindowTabWidget::displayRoomMessage(const Czateria::Message &msg) {
  mMainChatTab->appendPlainText(formatMessage(msg));
  indicateTabActivity(0, QIcon(QLatin1String(":/icons/transmit_blue.png")));
}

void ChatWindowTabWidget::displayPrivateMessage(const Czateria::Message &msg) {
  auto privMsgTab = privateMessageTab(msg.nickname());
  privMsgTab->appendPlainText(formatMessage(msg));
  indicateTabActivity(privMsgTab,
                      QIcon(QLatin1String(":/icons/transmit_blue.png")));
}

void ChatWindowTabWidget::openPrivateMessageTab(const QString &nickname) {
  setCurrentWidget(privateMessageTab(nickname));
}

ChatWindowTabWidget::PrivateChatTab *
ChatWindowTabWidget::privateMessageTab(const QString &nickname) {
  auto it = mPrivateTabs.find(nickname);
  if (it == std::end(mPrivateTabs)) {
    auto widget = PrivateChatTab::createAccepted(this, nickname);
    it = mPrivateTabs.insert(nickname, widget);
    addTab(widget, nickname);
  }
  return it.value();
}

int ChatWindowTabWidget::countUnreadPrivateTabs() const {
  int rv = 0;
  auto bar = tabBar();
  for (int i = 1; i < bar->count(); ++i) {
    if (bar->tabTextColor(i) == unreadTabColor) {
      ++rv;
    }
  }
  return rv;
}

void ChatWindowTabWidget::askAcceptPrivateMessage(const QString &nickname) {
  auto widget = PrivateChatTab::create(this, nickname);
  mPrivateTabs.insert(nickname, widget);
  addTab(widget, nickname);
}

void ChatWindowTabWidget::addMessageToPrivateChat(const QString &nickname,
                                                  const QString &str) {
  privateMessageTab(nickname)->appendPlainText(str);
}

void ChatWindowTabWidget::indicateTabActivity(int idx, const QIcon &icon) {
  if (idx == currentIndex()) {
    return;
  }
  tabBar()->setTabTextColor(idx, unreadTabColor);
  tabBar()->setTabIcon(idx, icon);
}

void ChatWindowTabWidget::indicateTabActivity(QWidget *tab, const QIcon &icon) {
  indicateTabActivity(indexOf(tab), icon);
}

void ChatWindowTabWidget::updateTabActivity(int idx) {
  tabBar()->setTabTextColor(idx, QColor());
  tabBar()->setTabIcon(idx, QIcon());
}

void ChatWindowTabWidget::onPrivateConversationStateChanged(
    const QString &nickname, Czateria::ConversationState state) {
  QString message;
  QIcon icon = QIcon(QLatin1String(":/icons/no_entry.png")); // common default
  using s = decltype(state);
  switch (state) {
  case s::Rejected:
    message = tr("User rejected the conversation request");
    break;
  case s::Closed:
    message = tr("User closed the conversation window");
    break;
  case s::UserLeft:
    message = tr("User logged out");
    icon = QIcon(QLatin1String(":/icons/door_out.png"));
    break;
  case s::NoPrivs:
    message = tr("User has turned off private conversations");
    break;
  case s::NoFreePrivs:
    message = tr("User has no free private conversation slots");
    break;
  default:
    Q_ASSERT(false);
    break;
  }
  writePrivateInfo(nickname, message, icon);
}

QString ChatWindowTabWidget::getCurrentNickname() const {
  auto curWidget = currentWidget();
  return curWidget == mMainChatTab
             ? QString()
             : static_cast<PrivateChatTab *>(curWidget)->nickname();
}

void ChatWindowTabWidget::addMessageToCurrent(const Czateria::Message &msg) {
  addMessageToCurrent(formatMessage(msg));
}

void ChatWindowTabWidget::addMessageToCurrent(const QString &str) {
  if (currentWidget() == mMainChatTab) {
    mMainChatTab->appendPlainText(str);
  } else {
    static_cast<PrivateChatTab *>(currentWidget())->appendPlainText(str);
  }
}

void ChatWindowTabWidget::onTabCloseRequested(int index) {
  if (index == 0) {
    return;
  }
  auto w = static_cast<PrivateChatTab *>(widget(index));
  auto nickname = w->nickname();
  removeTab(index);
  delete w;
  mPrivateTabs.remove(nickname);
  emit privateConversationClosed(nickname);
}

void ChatWindowTabWidget::writePrivateInfo(const QString &nickname,
                                           const QString &message,
                                           const QIcon &icon) {
  auto it = mPrivateTabs.find(nickname);
  if (it == std::end(mPrivateTabs)) {
    return;
  }
  it.value()->appendPlainText(
      QString(QLatin1String("[%1] %2"))
          .arg(QDateTime::currentDateTime().toString(QLatin1String("HH:mm:ss")))
          .arg(message));
  indicateTabActivity(it.value(), icon);
}
