#include "userinfo.h"

QString UserInfo::address() const
{
	return QStringLiteral("%1:%2")
			.arg(datagram.senderAddress().toString())
			.arg(datagram.senderPort());
}
