#include "chatwindowtabwidget.h"

#include <QApplication>
#include <QPlainTextEdit>
#include <QTabBar>

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

ChatWindowTabWidget::ChatWindowTabWidget(QWidget *parent) : QTabWidget(parent) {
  setTabBarAutoHide(true);
  setTabsClosable(true);
  addTab(createTextWidget(this), QObject::tr("Main chat"));
  connect(this, &QTabWidget::tabCloseRequested, this,
          &ChatWindowTabWidget::onTabCloseRequested);
  connect(this, &QTabWidget::currentChanged, this,
          &ChatWindowTabWidget::updateTabActivity);
}

void ChatWindowTabWidget::displayRoomMessage(const Czateria::Message &msg) {
  static_cast<QPlainTextEdit *>(widget(0))->appendPlainText(formatMessage(msg));
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

QPlainTextEdit *
ChatWindowTabWidget::privateMessageTab(const QString &nickname) {
  auto it = mPrivateTabs.find(nickname);
  if (it == std::end(mPrivateTabs)) {
    auto plaintext = createTextWidget(this);
    it = mPrivateTabs.insert(nickname, plaintext);
    addTab(plaintext, nickname);
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

void ChatWindowTabWidget::indicateTabActivity(int idx, const QIcon &icon) {
  if (idx == currentIndex()) {
    return;
  }
  tabBar()->setTabTextColor(idx, unreadTabColor);
  tabBar()->setTabIcon(idx, icon);
}

void ChatWindowTabWidget::indicateTabActivity(QPlainTextEdit *tab,
                                              const QIcon &icon) {
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
    message = QObject::tr("User rejected the conversation request");
    break;
  case s::Closed:
    message = QObject::tr("User closed the conversation window");
    break;
  case s::UserLeft:
    message = QObject::tr("User logged out");
    icon = QIcon(QLatin1String(":/icons/door_out.png"));
    break;
  case s::NoPrivs:
    message = QObject::tr("User has turned off private conversations");
    break;
  case s::NoFreePrivs:
    message = QObject::tr("User has no free private conversation slots");
    break;
  default:
    Q_ASSERT(false);
    break;
  }
  writePrivateInfo(nickname, message, icon);
}

QString ChatWindowTabWidget::getCurrentNickname() const {
  auto curIdx = currentIndex();
  return curIdx == 0 ? QString() : tabText(curIdx);
}

void ChatWindowTabWidget::addMessageToCurrent(const Czateria::Message &msg) {
  addMessageToCurrent(formatMessage(msg));
}

void ChatWindowTabWidget::addMessageToCurrent(const QString &str) {
  static_cast<QPlainTextEdit *>(currentWidget())->appendPlainText(str);
}

void ChatWindowTabWidget::onTabCloseRequested(int index) {
  if (index == 0) {
    return;
  }
  auto w = widget(index);
  auto nickname = tabText(index);
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
