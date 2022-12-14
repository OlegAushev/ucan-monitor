/**
 * @file purecan_controller.h
 * @author Oleg Aushev (aushevom@protonmail.com)
 * @brief 
 * @version 0.1
 * @date 2022-11-14
 * 
 * @copyright Copyright (c) 2022
 * 
 */


#pragma once


#include <thread>
#include <future>

#include "cansocket/cansocket.h"
#include "../purecan_def.h"
#include "purecan/device/purecan_device.h"


namespace purecan {


class Controller
{
private:
	std::shared_ptr<can::Socket> _socket;
	std::set<std::shared_ptr<IDevice>> _devices;
	std::map<canid_t, std::shared_ptr<IDevice>> _recvIdDeviceList;

	/* controller -> device */
	bool _isTxEnabled = true;
	struct TxMessageInfo
	{
		std::string_view name;
		std::chrono::milliseconds period;
		std::chrono::time_point<std::chrono::steady_clock> timepoint;
		std::function<can_payload_va(void)> creator; 
	};
	std::map<canid_t, TxMessageInfo> _txMessageList;

	/* THREADS */
	std::thread _threadRun;
	std::promise<void> _signalExitRunThread;
	void _run(std::future<void> futureExit);

	void _onFrameReceived(const can_frame& frame);
public:
	/**
	 * @brief Construct a new Controller object
	 * 
	 * @param socket 
	 */
	Controller(std::shared_ptr<can::Socket> socket);

	/**
	 * @brief Destroy the Controller object
	 * 
	 */
	~Controller();

	/**
	 * @brief 
	 * 
	 * @param device 
	 */
	void registerDevice(std::shared_ptr<IDevice> device);

	/**
	 * @brief 
	 * 
	 * @param id 
	 * @param name 
	 * @param period 
	 * @param creator 
	 */
	void registerTxMessage(canid_t id, std::string_view name, std::chrono::milliseconds period, std::function<can_payload_va(void)> creator)
	{
		_txMessageList.insert({id, {name, period, std::chrono::steady_clock::now(), creator}});
	}

	/**
	 * @brief 
	 * 
	 */
	void enableTxMessages() { _isTxEnabled = true; }
	
	/**
	 * @brief 
	 * 
	 */
	void disableTxMessages() { _isTxEnabled = false; }
};








} // namespace purecan


