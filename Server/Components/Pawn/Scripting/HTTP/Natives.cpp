#include <sdk.hpp>
#include "../Types.hpp"

struct PawnHTTPResponseHandler : HTTPResponseHandler {
	int index;
	String callback;
	PawnScript& script;

	PawnHTTPResponseHandler(int index, StringView callback, PawnScript& script) : index(index), callback(callback), script(script) {}

	void onHTTPResponse(int status, StringView body) override {
		// TODO async
		script.Call(callback, index, status, String(body));
	}
};

SCRIPT_API(HTTP, bool(int index, int method, std::string const& url, std::string const& data, std::string const& callback, PawnScript& script))
{
	PawnHTTPResponseHandler handler(index, callback, script);
	PawnManager::Get()->core->requestHTTP(handler, HTTPRequestType(method), url, data);
	return true;
}
