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
  QPlainTextEdit *privateMessageTab(const QString &nickname);
  int countUnreadPrivateTabs() const;
  void setUseEmoji(bool useEmoji) { mUseEmoji = useEmoji; }

signals:
  void privateConversationClosed(const QString &nickname);

private:
  void onTabCloseRequested(int index);
  void writePrivateInfo(const QString &nickname, const QString &message,
                        const QIcon &icon);
  void indicateTabActivity(int idx, const QIcon &icon);
  void indicateTabActivity(QPlainTextEdit *tab, const QIcon &icon);
  void updateTabActivity(int idx);
  QString formatMessage(const Czateria::Message &) const;

  QHash<QString, QPlainTextEdit *> mPrivateTabs;
  bool mUseEmoji;
};

#endif // CHATWINDOWTABWIDGET_H
