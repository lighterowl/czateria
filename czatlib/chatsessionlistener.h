#ifndef CHATSESSIONLISTENER_H
#define CHATSESSIONLISTENER_H

#include <QString>

namespace Czateria {
struct Room;
class Message;

/* an observer-like object added as an afterthought rather than conceived right
 * from the start. one can clearly see that it duplicates some of the
 * functionality exposed in ChatSession as signals.
 * a very important distinction is that this listener's functions are called for
 * messages coming both ways : the ones received as well as the ones sent
 * locally. */
struct ChatSessionListener {
  virtual ~ChatSessionListener() = 0;
  virtual void onRoomMessage(const Room &room, const Message &message) = 0;
  virtual void onPrivateMessageReceived(const Room &room,
                                        const Message &message) = 0;
  virtual void onPrivateMessageSent(const Room &room,
                                    const Message &message) = 0;
  virtual void onUserJoined(const Room &room, const QString &nickname) = 0;
  virtual void onUserLeft(const Room &room, const QString &nickname) = 0;
};
} // namespace Czateria

#endif // CHATSESSIONLISTENER_H
