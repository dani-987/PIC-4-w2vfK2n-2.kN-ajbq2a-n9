#include "Backend.h"

#include <cstring>

#define NOT_IMPLEMENTED	"Nicht implementiert!"

#define MSGLEN()	(lastmsgLen = (std::strlen(lastmsg)+1))

Backend::Backend()
{
}


Backend::~Backend()
{
}

bool Backend::LoadProgramm(char * c)
{
	m_lastmsg.lock();
	lastmsg = NOT_IMPLEMENTED;
	MSGLEN();
	m_lastmsg.unlock();
	return false;
}

bool Backend::Start()
{
	m_lastmsg.lock();
	lastmsg = NOT_IMPLEMENTED;
	MSGLEN();
	m_lastmsg.unlock();
	return false;
}

bool Backend::Stop()
{
	m_lastmsg.lock();
	lastmsg = NOT_IMPLEMENTED;
	MSGLEN();
	m_lastmsg.unlock();
	return false;
}

bool Backend::Step()
{
	m_lastmsg.lock();
	lastmsg = NOT_IMPLEMENTED;
	MSGLEN();
	m_lastmsg.unlock();
	return false;
}

bool Backend::Reset()
{
	m_lastmsg.lock();
	lastmsg = NOT_IMPLEMENTED;
	MSGLEN();
	m_lastmsg.unlock();
	return false;
}

bool Backend::SetMem(int from, int len, void * mem)
{
	m_lastmsg.lock();
	lastmsg = NOT_IMPLEMENTED;
	MSGLEN();
	m_lastmsg.unlock();
	return false;
}

bool Backend::SetBit(int byte, int pos, bool val)
{
	m_lastmsg.lock();
	lastmsg = NOT_IMPLEMENTED;
	MSGLEN();
	m_lastmsg.unlock();
	return false;
}

void * Backend::GetMem(int from, int len)
{
	m_lastmsg.lock();
	lastmsg = NOT_IMPLEMENTED;
	MSGLEN();
	m_lastmsg.unlock();
	return nullptr;
}

int Backend::GetBit(int byte, int pos)
{
	m_lastmsg.lock();
	lastmsg = NOT_IMPLEMENTED;
	MSGLEN();
	m_lastmsg.unlock();
	return -1;
}

int Backend::getRegW()
{
	m_lastmsg.lock();
	lastmsg = NOT_IMPLEMENTED;
	MSGLEN();
	m_lastmsg.unlock();
	return -1;
}

bool Backend::setRegW(char val)
{
	m_lastmsg.lock();
	lastmsg = NOT_IMPLEMENTED;
	MSGLEN();
	m_lastmsg.unlock();
	return false;
}

char * Backend::getErrorMSG()
{
	m_lastmsg.lock();
	if (lastmsg == nullptr)return nullptr;
	char* ret = (char*)malloc(lastmsgLen*sizeof(char));
	memcpy(ret, lastmsg, lastmsgLen * sizeof(char));
	m_lastmsg.unlock();
	return ret;
}
