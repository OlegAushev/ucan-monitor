/**
 * @file bmsmain_server.h
 * @author Oleg Aushev (aushevom@protonmail.com)
 * @brief 
 * @version 0.1
 * @date 2022-12-23
 * 
 * @copyright Copyright (c) 2022
 * 
 */


#pragma once


#include "ucanopen/server/ucanopen_server.h"
#include "bmsmain_def.h"


namespace bmsmain {


class Server : public ucanopen::IServer
{
private:
	double _current = 0;
	double _voltage = 0;
	unsigned int _charge = 0;
	double _tempMin = 0;
	double _tempMax = 0;

protected:
	virtual void _handleTpdo1(ucanopen::can_payload data) override final;
	virtual void _handleTpdo2(ucanopen::can_payload data) override final {}
	virtual void _handleTpdo3(ucanopen::can_payload data) override final {}
	virtual void _handleTpdo4(ucanopen::can_payload data) override final {}

	virtual ucanopen::can_payload _createRpdo1() override final { return {}; }
	virtual ucanopen::can_payload _createRpdo2() override final { return {}; }
	virtual ucanopen::can_payload _createRpdo3() override final { return {}; }
	virtual ucanopen::can_payload _createRpdo4() override final { return {}; }

	virtual void _handleTsdo(ucanopen::SdoType sdoType,
			ucanopen::ObjectDictionary::const_iterator entryIt,
			ucanopen::CobSdoData data) override final {}

public:
	Server(const std::string& name, ucanopen::NodeId nodeId, std::shared_ptr<can::Socket> socket);

	double current() const { return _current; }
	double voltage() const { return _voltage; }
	unsigned int charge() const { return _charge; }
	double tempMin() const { return _tempMin; }
	double tempMax() const { return _tempMax; }
};


}


