#include "timers.hpp"

Pair<size_t, PawnTimerHandler*> PawnTimerImpl::newTimer(const char* callback, Milliseconds interval, bool repeating, AMX* amx) {
	ITimersComponent* timers = PawnManager::Get()->timers;
	if (timers && amx) {
		int funcidx;
		if (amx_FindPublic(amx, callback, &funcidx) != AMX_ERR_NONE)
		{
			PawnManager::Get()->core->logLn(LogLevel::Warning, "SetTimer(Ex): Couldn't find %s in your script", callback);
		}
		else {
			PawnTimerHandler* handler = new PawnTimerHandler(funcidx, amx);
			ITimer* timer = timers->create(handler, interval, repeating);
			if (timer == nullptr) {
				delete handler;
			}
			else {
				size_t idx = insert(timer);
				handler->poolID = idx;
				return std::make_pair(idx, handler);
			}
		}
	}
	return std::make_pair(0u, static_cast<PawnTimerHandler*>(nullptr));
}

int PawnTimerImpl::setTimer(const char* callback, Milliseconds interval, bool repeating, AMX* amx) {
	return newTimer(callback, interval, repeating, amx).first;
}

int PawnTimerImpl::setTimerEx(const char* callback, Milliseconds interval, bool repeating, const char* fmt, AMX* amx, const cell* params) {
	auto res = newTimer(callback, interval, repeating, amx);
	PawnTimerHandler* handler = res.second;
	if (res.second) {
		handler->fmt = fmt;

		cell* data;
		cell* len1;
		int len2;

		// Collect data and parameters
		for (size_t i = 0; fmt[i]; ++i)
		{
			switch (fmt[i])
			{
			case 'a':
				++i;
				if (fmt[i] != 'i' && fmt[i] != 'd')
				{
					PawnManager::Get()->core->logLn(LogLevel::Error, "SetTimerEx: Error in pushing parameters; Array not followed by size");
					return 0;
				}
				if (
					amx_GetAddr(amx, params[i - 1], &data) != AMX_ERR_NONE ||
					amx_GetAddr(amx, params[i], &len1) != AMX_ERR_NONE ||
					*len1 < 1)
				{
					PawnManager::Get()->core->logLn(LogLevel::Error, "SetTimerEx: Error in pushing parameters");
					return 0;
				}
				// Store the offset in to the new heap data, then the size, then copy the data.
				handler->params.push_back(handler->data.size() * sizeof(cell));
				handler->params.push_back(*len1);
				handler->data.insert(handler->data.end(), data, data + *len1);
				break;
			case 's':
				if (amx_GetAddr(amx, params[i], &data) != AMX_ERR_NONE || amx_StrSize(data, &len2) != AMX_ERR_NONE)
				{
					PawnManager::Get()->core->logLn(LogLevel::Error, "SetTimerEx: Error in pushing parameters");
					return 0;
				}
				// Store the offset in to the new heap data, then copy the data.
				handler->params.push_back(handler->data.size() * sizeof(cell));
				handler->data.insert(handler->data.end(), data, data + len2);
				break;
			case 'v':
				if (amx_GetAddr(amx, params[i], &data) != AMX_ERR_NONE)
				{
					PawnManager::Get()->core->logLn(LogLevel::Error, "SetTimerEx: Error in pushing parameters");
					return 0;
				}
				// Store the offset in to the new heap data, then copy the data.
				handler->params.push_back(handler->data.size() * sizeof(cell));
				handler->data.push_back(*data);
				break;
			default:
				if (amx_GetAddr(amx, params[i], &data) != AMX_ERR_NONE)
				{
					PawnManager::Get()->core->logLn(LogLevel::Error, "SetTimerEx: Error in pushing parameters");
					return 0;
				}
				handler->params.push_back(*data);
				break;
			}
		}
	}
	return res.first;
}
