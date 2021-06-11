#ifndef CHATWINDOWTABWIDGET_H
#define CHATWINDOWTABWIDGET_H

#include <QHash>
#include <QTabWidget>

#include <czatlib/conversationstate.h>

namespace Czateria {
class Message;
}

class QPlainTextEdit;

class ChatWindowTabWidget : public QTabWidget {
  Q_OBJECT
public:
  ChatWindowTabWidget(QWidget *parent = nullptr);

  void displayRoomMessage(const Czateria::Message &msg);
  void displayPrivateMessage(const Czateria::Message &msg);
  void openPrivateMessageTab(const QString &nickname);
  void onPrivateConversationStateChanged(const QString &nickname,
                                         Czateria::ConversationState state);
  QString getCurrentNickname() const;

  void addMessageToCurrent(const Czateria::Message &msg);
  void addMessageToCurrent(const QString &str);
  int countUnreadPrivateTabs() const;
  void askAcceptPrivateMessage(const QString &nickname);
  void addMessageToPrivateChat(const QString &nickname, const QString &str);
  bool privTabIsOpen(const QString &nickname) const {
    return mPrivateTabs.contains(nickname);
  }
  void setUseEmoji(bool useEmoji) { mUseEmoji = useEmoji; }
  bool shouldUseEmoji() const { return mUseEmoji; }
  void closePrivateConversationTab(const QString &nickname);
  void writeConversationState(const QString &nickname, const QString &message,
                              const QIcon &icon);

signals:
  void privateConversationAccepted(const QString &nickname);
  void privateConversationRejected(const QString &nickname);
  void privateConversationClosed(const QString &nickname, int tabIndex);

private:
  void onTabCloseRequested(int index);
  void indicateTabActivity(int idx, const QIcon &icon);
  void indicateTabActivity(QWidget *tab, const QIcon &icon);
  void updateTabActivity(int idx);
  QString formatMessage(const Czateria::Message &) const;

  class PrivateChatTab;
  void writePrivateInfo(PrivateChatTab *, const QString &message,
                        const QIcon &icon);
  PrivateChatTab *privateMessageTab(const QString &nickname);

  QPlainTextEdit *const mMainChatTab;
  QHash<QString, PrivateChatTab *> mPrivateTabs;
  bool mUseEmoji;
};

#endif // CHATWINDOWTABWIDGET_H
