/**
 * @file launchpad_server.h
 * @author Oleg Aushev (aushevom@protonmail.com)
 * @brief 
 * @version 0.1
 * @date 2022-12-12
 * 
 * @copyright Copyright (c) 2022
 * 
 */


#pragma once


#include "ucanopen/server/ucanopen_server.h"
#include "launchpad_def.h"
#include "logger/logger.h"


namespace launchpad {


extern const ucanopen::ObjectDictionary objectDictionary;
extern const ucanopen::ObjectDictionaryConfig objectDictionaryConfig;


class Server : public ucanopen::IServer
{
protected:
	virtual void _handleTpdo1(ucanopen::can_payload data) override final {}
	virtual void _handleTpdo2(ucanopen::can_payload data) override final {}
	virtual void _handleTpdo3(ucanopen::can_payload data) override final {}
	virtual void _handleTpdo4(ucanopen::can_payload data) override final {}

	virtual ucanopen::can_payload _createRpdo1() override final { return {}; }
	virtual ucanopen::can_payload _createRpdo2() override final { return {}; }
	virtual ucanopen::can_payload _createRpdo3() override final { return {}; }
	virtual ucanopen::can_payload _createRpdo4() override final { return {}; }

	virtual void _handleTsdo(ucanopen::SdoType sdoType,
			ucanopen::ObjectDictionary::const_iterator entryIt,
			ucanopen::CobSdoData data) override final;

public:
	Server(const std::string& name,	ucanopen::NodeId nodeId, std::shared_ptr<can::Socket> socket);
};


} // namespace launchpad


