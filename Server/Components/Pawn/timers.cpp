#include "timers.hpp"

Pair<size_t, PawnTimerHandler*> PawnTimerImpl::newTimer(const char* callback, Milliseconds interval, bool repeating, AMX* amx) {
	ITimersComponent* timers = PawnManager::Get()->timers;
	if (timers && amx) {
		int funcidx;
		int err = AMX_ERR_NONE;
		if ((err = amx_FindPublic(amx, callback, &funcidx)) != AMX_ERR_NONE)
		{
			PawnManager::Get()->core->logLn(LogLevel::Warning, "SetTimer(Ex): Couldn't find %s in your script: %s", callback, aux_StrError(err));
			amx_RaiseError(amx, err);
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
		int err = AMX_ERR_NONE;
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
					err = AMX_ERR_PARAMS;
					return newTimerExError(handler, amx, err, "Error in pushing parameters; Array not followed by size");
				}
				if (
					(err = amx_GetAddr(amx, params[i - 1], &data)) != AMX_ERR_NONE ||
					(err = amx_GetAddr(amx, params[i], &len1)) != AMX_ERR_NONE ||
					*len1 < 1)
				{
					if (err == AMX_ERR_NONE) {
						err = AMX_ERR_PARAMS;
					}
					return newTimerExError(handler, amx, err, "Error in pushing parameters");
				}
				// Store the offset in to the new heap data, then the size, then copy the data.
				handler->params.push_back(handler->data.size() * sizeof(cell));
				handler->params.push_back(*len1);
				handler->data.insert(handler->data.end(), data, data + *len1);
				break;
			case 's':
				if ((err = amx_GetAddr(amx, params[i], &data)) != AMX_ERR_NONE || (err = amx_StrSize(data, &len2)) != AMX_ERR_NONE)
				{
					return newTimerExError(handler, amx, err, "Error in pushing parameters");
				}
				// Store the offset in to the new heap data, then copy the data.
				handler->params.push_back(handler->data.size() * sizeof(cell));
				handler->data.insert(handler->data.end(), data, data + len2);
				break;
			case 'v':
				if ((err = amx_GetAddr(amx, params[i], &data)) != AMX_ERR_NONE)
				{
					return newTimerExError(handler, amx, err, "Error in pushing parameters");
				}
				// Store the offset in to the new heap data, then copy the data.
				handler->params.push_back(handler->data.size() * sizeof(cell));
				handler->data.push_back(*data);
				break;
			default:
				if ((err = amx_GetAddr(amx, params[i], &data)) != AMX_ERR_NONE)
				{
					return newTimerExError(handler, amx, err, "Error in pushing parameters");
				}
				handler->params.push_back(*data);
				break;
			}
		}
	}
	return res.first;
}

int PawnTimerImpl::newTimerExError(PawnTimerHandler* handler, AMX* amx, int err, StringView message) {
	amx_RaiseError(amx, err);
	PawnManager::Get()->core->logLn(LogLevel::Error, "SetTimerEx: %s: %s", message.data(), aux_StrError(err));
	delete handler;
	return 0;
}

void PawnTimerImpl::killTimers(AMX* amx) {
	for (auto& kv : pool) {
		PawnTimerHandler* handler = static_cast<PawnTimerHandler*>(kv.second->handler());
		if (handler->amx == amx) {
			kv.second->kill();
		}
	}
}
