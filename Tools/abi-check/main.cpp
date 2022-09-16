#include <libelfin/elf/elf++.hh>
#include <libelfin/dwarf/dwarf++.hh>
#include <cxxopts.hpp>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <map>
#include <vector>
#include <assert.h>
#include <memory>
#include <string>
#include <stdexcept>
#include <regex>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat-security"
template<typename ... Args>
std::string string_format(const std::string& format, Args ... args)
{
    int size_s = std::snprintf( nullptr, 0, format.c_str(), args ... ) + 1; // Extra space for '\0'
    if( size_s <= 0 ){ throw std::runtime_error( "Error during formatting." ); }
    auto size = static_cast<size_t>( size_s );
    std::unique_ptr<char[]> buf( new char[ size ] );
    std::snprintf( buf.get(), size, format.c_str(), args ... );
    return std::string( buf.get(), buf.get() + size - 1 ); // We don't want the '\0' inside
}
#pragma clang diagnostic pop

using namespace dwarf;

/* DWARF calling conventions encodings.
   Used as values of DW_AT_calling_convention for subroutines
   (normal, program or nocall) or structures, unions and class types
   (normal, reference or value).  */
enum class DW_CC_DWARF5
{
    normal = 0x1,
    program = 0x2,
    nocall = 0x3,
    pass_by_reference = 0x4,
    pass_by_value = 0x5,
    lo_user = 0x40,
    hi_user = 0xff
};

std::string to_string(DW_CC_DWARF5 cc) {
    if (cc == DW_CC_DWARF5::normal) {
        return "normal";
    }
    if (cc == DW_CC_DWARF5::program) {
        return "program";
    }
    if (cc == DW_CC_DWARF5::nocall) {
        return "nocall";
    }
    if (cc == DW_CC_DWARF5::pass_by_reference) {
        return "byref";
    }
    if (cc == DW_CC_DWARF5::pass_by_value) {
        return "byval";
    }
    if (cc == DW_CC_DWARF5::lo_user) {
        return "louser";
    }
    if (cc == DW_CC_DWARF5::hi_user) {
        return "hiuser";
    }
    return std::string();
}

struct Settings {
    std::regex incl = std::regex(".*");
    std::regex excl;
    bool names = true;
};

struct TypeInfo {
    std::string name;
    std::string output;
    uint64_t size = 0;
    bool isEmpty = true;
};

const TypeInfo& process_type(const Settings& settings, const die& node);

std::map<section_offset, uint64_t> cacheIndices;

// We want the cache to be ordered so the same input is yielded if no ABI break occurred
std::map<uint64_t, TypeInfo> cache;
uint64_t cacheOrder = 0;

TypeInfo& newCacheEntry(const die &node)
{
    section_offset offset = node.get_section_offset();
    auto cacheIndicesIt = cacheIndices.find(offset);
    cacheIndices.emplace(offset, ++cacheOrder);
    return cache.emplace(cacheOrder, TypeInfo()).first->second;
}

const TypeInfo PtrTypeInfo{ "pointer", "", sizeof(void*) };
const TypeInfo FnPtrTypeInfo{ "fnpointer", "", sizeof(void*)*2 };
const TypeInfo VoidTypeInfo {"void", "", 0 };

void process_virtual(const Settings& settings, const die& node, TypeInfo &ti)
{
    const TypeInfo& retType = node.has(DW_AT::type) ? process_type(settings, node.resolve(DW_AT::type).as_reference()) : VoidTypeInfo;

    uint64_t index = node.resolve(DW_AT::vtable_elem_location).as_exprloc().evaluate(&no_expr_context, 0).value;
    ti.output += string_format("info=\"virtual\" index=%llu retsize=%llu", index, retType.size);
    if (settings.names) {
        ti.output += string_format(" returns=\"%s\" name=\"%s\"", retType.name.c_str(), node.resolve(DW_AT::name).as_cstr());
    }
    ti.output += "\n";

    uint64_t pindex = 0;
    for (auto it=node.begin(); it!=node.end(); ++it) {
        if (it->tag==DW_TAG::formal_parameter) {
            const char* pname = it->has(DW_AT::name) ? it->resolve(DW_AT::name).as_cstr() : nullptr;
            if (!pname && pindex==0 && it->has(DW_AT::artificial) && it->resolve(DW_AT::artificial).as_flag()) {
                pname = "this";
            }
            const TypeInfo& retType = it->has(DW_AT::type) ? process_type(settings, it->resolve(DW_AT::type).as_reference()) : VoidTypeInfo;
            ti.output += string_format("info=\"param\" index=%llu size=%llu", pindex++, retType.size);
            if (settings.names) {
                ti.output += string_format(" type=\"%s\"", retType.name.c_str());
                if (pname) {
                    ti.output += string_format(" name=\"%s\"", pname);
                }
            }
            ti.output += "\n";
        }
    }

    ti.output += string_format("info=\"endvirtual\" args=%llu\n", pindex);
}

const TypeInfo& process_union(const Settings& settings, const die& node)
{
    TypeInfo& ti = newCacheEntry(node);
    ti.size = node.resolve(DW_AT::byte_size).as_uconstant();
    ti.name = node.has(DW_AT::name) ? node.resolve(DW_AT::name).as_string() : string_format("anonymous union %llu", cacheOrder);
    ti.output += string_format("info=\"union\" size=%llu", ti.size);
    if (node.has(DW_AT::calling_convention)) {
        uint64_t cc = node.resolve(DW_AT::calling_convention).as_uconstant();
        ti.output += string_format(" call=\"%s\"", to_string(DW_CC_DWARF5(cc)).c_str());
    }
    if (settings.names) {
        ti.output += string_format(" name=\"%s\"", ti.name.c_str());
    }
    ti.output += "\n";

    for (auto it=node.begin(); it!=node.end(); ++it) {
        if (it->tag == DW_TAG::member && (!it->has(DW_AT::declaration) || !it->resolve(DW_AT::declaration).as_flag())) {
            die subtype = it->resolve(DW_AT::type).as_reference();
            process_type(settings, subtype);
            ti.isEmpty = false;
        }
    }

    ti.output += string_format("info=\"endunion\"\n");

    return ti;
}

const TypeInfo& process_array(const Settings& settings, const die& node)
{
    TypeInfo& ti = newCacheEntry(node);
    ti = process_type(settings, node.resolve(DW_AT::type).as_reference());
    for (auto it=node.begin(); it!=node.end(); ++it) {
        if (it->tag == DW_TAG::subrange_type) {
            // not sure if to reverse when printing
            const uint64_t cnt = it->resolve(DW_AT::count).as_uconstant();
            ti.name += string_format("[%llu]", cnt);
            ti.size *= cnt;
        }
    }

    return ti;
}

const TypeInfo& process_type(const Settings& settings, const die& node)
{
    // Don't process forward declared classes
    if (node.has(DW_AT::declaration) && node.resolve(DW_AT::declaration).as_flag()) {
        return VoidTypeInfo;
    }
    // Pointer type size
    if (node.tag==DW_TAG::pointer_type || node.tag==DW_TAG::reference_type || node.tag==DW_TAG::rvalue_reference_type) {
        return PtrTypeInfo;
    }
    if (node.tag==DW_TAG::ptr_to_member_type) {
        // Member function size is ptr*2
        if (node.has(DW_AT::type) && node.resolve(DW_AT::type).as_reference().tag==DW_TAG::subroutine_type) {
            return FnPtrTypeInfo;
        } else {
            return PtrTypeInfo;
        }
    }

    // Cache lookup
    section_offset offset = node.get_section_offset();
    auto cacheIndicesIt = cacheIndices.find(offset);
    if (cacheIndicesIt != cacheIndices.end()) {
        assert(cache.find(cacheIndicesIt->second) != cache.end());
        return cache.find(cacheIndicesIt->second)->second;
    }

    // Typedef forward
    if (node.tag==DW_TAG::typedef_) {
        TypeInfo& ti = newCacheEntry(node);
        ti = process_type(settings, node.resolve(DW_AT::type).as_reference());
        ti.name = node.resolve(DW_AT::name).as_string();
        ti.isEmpty = true; // Set to true so typedefs aren't printed
        return ti;
    }

    // Const type from type
    if (node.tag==DW_TAG::const_type) {
        TypeInfo& ti = newCacheEntry(node);
        ti = process_type(settings, node.resolve(DW_AT::type).as_reference());
        ti.name += " const";
        return ti;
    }

    // Array type from type
    if (node.tag==DW_TAG::array_type) {
        return process_array(settings, node);
    }

    // Union type
    if (node.tag==DW_TAG::union_type) {
        return process_union(settings, node);
    }

    std::vector<die> inheritance;
    std::vector<die> virtuals;
    std::vector<die> members;

    TypeInfo& ti = newCacheEntry(node);
    ti.name = node.has(DW_AT::name) ? node.resolve(DW_AT::name).as_string() : string_format("anonymous struct %llu", cacheOrder);
    if (node.has(DW_AT::byte_size)) {
        ti.size = node.resolve(DW_AT::byte_size).as_uconstant();
    }

    ti.output += string_format("info=\"struct\" size=%llu", ti.size);
    if (node.has(DW_AT::calling_convention)) {
        uint64_t cc = node.resolve(DW_AT::calling_convention).as_uconstant();
        ti.output += string_format(" call=\"%s\"", to_string(DW_CC_DWARF5(cc)).c_str());
    }
    if (settings.names) {
        ti.output += string_format(" name=\"%s\"", ti.name.c_str());
    }
    ti.output += "\n";

    for (auto it=node.begin(); it!=node.end(); ++it) {
        // Virtuals
        if (it->tag == DW_TAG::subprogram && it->has(DW_AT::virtuality) &&
            DW_VIRTUALITY(it->resolve(DW_AT::virtuality).as_uconstant()) != DW_VIRTUALITY::none
            && it->has(DW_AT::declaration) && it->resolve(DW_AT::declaration).as_flag()
        ) {
            virtuals.push_back(*it);
        }
        // Members
        if (it->tag == DW_TAG::member && it->has(DW_AT::data_member_location)) {
            members.push_back(*it);
        }
        // Inheritance
        if (it->tag == DW_TAG::inheritance) {
            inheritance.push_back(*it);
        }
    }

    if (!inheritance.empty() || !members.empty() || !virtuals.empty()) {
        ti.isEmpty = false;
    }

    for (auto it=inheritance.begin(); it!=inheritance.end(); ++it) {
        die subtype = it->resolve(DW_AT::type).as_reference();
        TypeInfo res = process_type(settings, subtype);

        ti.output += string_format("info=\"inherits\" offset=%llu size=%llu", it->resolve(DW_AT::data_member_location).as_uconstant(), res.size);
        if (settings.names) {
            ti.output += string_format(" type=\"%s\"", res.name.c_str());
        }
        ti.output += "\n";
    }

    for (auto it=members.begin(); it!=members.end(); ++it) {
        die subtype = it->resolve(DW_AT::type).as_reference();
        const char* subname = it->has(DW_AT::name) ? it->resolve(DW_AT::name).as_cstr() : "";
        TypeInfo res = process_type(settings, subtype);

        ti.output += string_format("info=\"member\" offset=%llu size=%llu", it->resolve(DW_AT::data_member_location).as_uconstant(), res.size);
        if (settings.names) {
            ti.output += string_format(" type=\"%s\" name=\"%s\"", res.name.c_str(), subname);
        }
        ti.output += "\n";
    }

    for (auto it=virtuals.begin(); it!=virtuals.end(); ++it) {
        process_virtual(settings, *it, ti);
    }

    ti.output += string_format("info=\"endstruct\" virtuals=%u\n", virtuals.size());

    return ti;
}

void collect(const Settings& settings, const line_table& lines, const die& node, std::multimap<uint64_t, die>& output)
{
    if (node.has(DW_AT::name)) {
        if (node.tag==DW_TAG::structure_type || node.tag==DW_TAG::class_type || node.tag==DW_TAG::union_type) {
            if (node.has(DW_AT::decl_file)) {
                uint64_t index = node.resolve(DW_AT::decl_file).as_uconstant();
                const line_table::file* f = lines.get_file(index);
                if (f) {
                    if (std::regex_search(f->path, settings.incl) && !std::regex_search(f->path, settings.excl)) {
                        uint64_t size = node.has(DW_AT::byte_size) ? node.resolve(DW_AT::byte_size).as_uconstant() : 0u;
                        output.emplace(size, node);
                    }
                }
            }
        }
    }

    for (auto &child : node) {
        collect(settings, lines, child, output);
    }
}

int main(int argc, char** argv)
{
    Settings settings;

    cxxopts::Options options("abi-check", "[--incl <regex>]\n[--excl <regex>]\n[--no-names]\n<binary>");
    options.add_options()
        ("incl", "Include path regex", cxxopts::value<std::string>())
        ("excl", "Exclude path regex", cxxopts::value<std::string>())
        ("no-names", "Don't display names", cxxopts::value<bool>()->default_value("false"))
        ("binary", "The binary to process", cxxopts::value<std::string>());

    options.parse_positional("binary");

    auto result = options.parse(argc, argv);

    std::string fname = result["binary"].as<std::string>();

    if (result.count("incl")) {
        settings.incl = std::regex(result["incl"].as<std::string>(), std::regex::nosubs | std::regex::optimize | std::regex::ECMAScript);
    }

    if (result.count("excl")) {
        settings.excl = std::regex(result["excl"].as<std::string>(), std::regex::nosubs | std::regex::optimize | std::regex::ECMAScript);
    }

    settings.names = !result["no-names"].as<bool>();

    int fd = open(fname.c_str(), O_RDONLY);
    if (fd < 0) {
        fprintf(stderr, "%s: %s\n", argv[1], strerror(errno));
        return 1;
    }

    ::elf::elf ef(::elf::create_mmap_loader(fd));
    ::dwarf::dwarf dw(::dwarf::elf::create_loader(ef));
    for (auto cu : dw.compilation_units()) {
        // Sort leaves by size so the same input is yielded with different DWARF order
        std::multimap<uint64_t, die> structs;
        collect(settings, cu.get_line_table(), cu.root(), structs);
        for (auto it = structs.begin(); it != structs.end(); ++it) {
            process_type(settings, it->second);
        }
    }

    close(fd);

    for (auto& v : cache) {
        if (!v.second.isEmpty) {
            printf("%s", v.second.output.c_str());
        }
    }

    return 0;
}
