#include <Server/Components/Unicode/unicode.hpp>
#include <sdk.hpp>
#include <unicode/ucsdet.h>
#include <unicode/unistr.h>

struct UnicodeComponent final : public IUnicodeComponent {
    ICore* core;

    void onLoad(ICore* core) override
    {
    }

    OptimisedString toUTF8(StringView input) override
    {
        static UErrorCode detstatus = U_ZERO_ERROR;
        static UCharsetDetector* detector = ucsdet_open(&detstatus);
        if (U_FAILURE(detstatus)) {
            return OptimisedString(input);
        }
        UErrorCode status = U_ZERO_ERROR;
        ucsdet_setText(detector, input.data(), input.length(), &status);
        const char* cp = ucsdet_getName(ucsdet_detect(detector, &status), &status);
        if (U_FAILURE(status)) {
            return OptimisedString(input);
        }
        String output;
        icu::UnicodeString(input.data(), input.length(), cp).toUTF8String(output);
        return OptimisedString(output);
    }

    StringView componentName() const override
    {
        return "Unicode";
    }

    SemanticVersion componentVersion() const override
    {
        return SemanticVersion(0, 0, 0, BUILD_NUMBER);
    }

    void free() override
    {
        delete this;
    }
};

COMPONENT_ENTRY_POINT()
{
    return new UnicodeComponent();
}
