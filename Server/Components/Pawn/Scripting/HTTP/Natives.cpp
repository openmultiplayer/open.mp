#include "../Types.hpp"
#include <sdk.hpp>

struct PawnHTTPResponseHandler final : HTTPResponseHandler {
    int index;
    String callback;
    PawnScript& script;

    PawnHTTPResponseHandler(int index, StringView callback, PawnScript& script)
        : index(index)
        , callback(callback)
        , script(script)
    {
    }

    void onHTTPResponse(int status, StringView body) override
    {
        script.Call(callback, DefaultReturnValue_True, index, status, String(body));
        delete this;
    }
};

SCRIPT_API(HTTP, bool(int index, int method, std::string const& url, std::string const& data, std::string const& callback, PawnScript& script))
{
    PawnManager::Get()->core->requestHTTP(new PawnHTTPResponseHandler(index, callback, script), HTTPRequestType(method), url, data);
    return true;
}
