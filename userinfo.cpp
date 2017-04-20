#include "userinfo.h"
#include "datasynccontrol.h"

UserInfo::UserInfo() :
	name(),
	datagram()
{}

QString UserInfo::address() const
{
	return QStringLiteral("%1:%2")
			.arg(datagram.senderAddress().toString())
			.arg(datagram.senderPort());
}
