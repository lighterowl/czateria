#ifndef CONVERSATIONSTATE_H
#define CONVERSATIONSTATE_H

namespace Czateria {
enum class ConversationState {
  InviteSent,
  InviteReceived,
  Active,
  Rejected,
  Closed,
  NoPrivs,
  NoFreePrivs,
  UserLeft
};
}

#endif // CONVERSATIONSTATE_H
