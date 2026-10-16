/**
 * @file FormSerializer.cpp
 * @brief FormSerializer 實作 (確定性 JSON 序列化、預設值剔除與容錯反序列化)
 */

#include "ui/designer/serialization/FormSerializer.hpp"
#include "ui/designer/serialization/SchemaVersion.hpp"
#include "ui/designer/ast/FormDocumentNode.hpp"
#include "ui/designer/metadata/PropertyDescriptor.hpp"
#include "designer/serialization/JsonParserInternal.hpp"
#include <sstream>
#include <iomanip>

namespace cppui {
namespace designer {

using namespace detail;

namespace {

std::string ColorToHex(const core::Color& c) {
    std::ostringstream ss;
    ss << "#"
       << std::hex << std::setfill('0')
       << std::setw(2) << static_cast<uint32_t>(c.a)
       << std::setw(2) << static_cast<uint32_t>(c.r)
       << std::setw(2) << static_cast<uint32_t>(c.g)
       << std::setw(2) << static_cast<uint32_t>(c.b);
    return ss.str();
}

compat::optional<core::Color> HexToColor(compat::string_view hex) {
    if (hex.size() != 9 || hex[0] != '#') {
        return compat::nullopt;
    }
    try {
        uint32_t val = static_cast<uint32_t>(std::stoul(std::string(hex.substr(1)), nullptr, 16));
        uint8_t a = static_cast<uint8_t>((val >> 24) & 0xFF);
        uint8_t r = static_cast<uint8_t>((val >> 16) & 0xFF);
        uint8_t g = static_cast<uint8_t>((val >> 8) & 0xFF);
        uint8_t b = static_cast<uint8_t>(val & 0xFF);
        return core::Color{ r, g, b, a };
    } catch (...) {
        return compat::nullopt;
    }
}

JsonValue PropertyValueToJson(const PropertyValue& val) {
    const auto& var = val.GetVariant();

    if (const bool* b = std::get_if<bool>(&var)) {
        return JsonValue(*b);
    }
    if (const int32_t* i32 = std::get_if<int32_t>(&var)) {
        return JsonValue(*i32);
    }
    if (const int64_t* i64 = std::get_if<int64_t>(&var)) {
        return JsonValue(*i64);
    }
    if (const double* d = std::get_if<double>(&var)) {
        return JsonValue(*d);
    }
    if (const std::string* s = std::get_if<std::string>(&var)) {
        return JsonValue(*s);
    }
    if (const core::Point* pt = std::get_if<core::Point>(&var)) {
        JsonValue::ObjectType obj;
        obj["x"] = JsonValue(pt->x);
        obj["y"] = JsonValue(pt->y);
        return JsonValue(std::move(obj));
    }
    if (const core::Size* sz = std::get_if<core::Size>(&var)) {
        JsonValue::ObjectType obj;
        obj["height"] = JsonValue(sz->height);
        obj["width"] = JsonValue(sz->width);
        return JsonValue(std::move(obj));
    }
    if (const core::Rect* rc = std::get_if<core::Rect>(&var)) {
        JsonValue::ObjectType obj;
        obj["height"] = JsonValue(rc->height);
        obj["width"] = JsonValue(rc->width);
        obj["x"] = JsonValue(rc->x);
        obj["y"] = JsonValue(rc->y);
        return JsonValue(std::move(obj));
    }
    if (const core::Thickness* th = std::get_if<core::Thickness>(&var)) {
        JsonValue::ObjectType obj;
        obj["bottom"] = JsonValue(th->bottom);
        obj["left"] = JsonValue(th->left);
        obj["right"] = JsonValue(th->right);
        obj["top"] = JsonValue(th->top);
        return JsonValue(std::move(obj));
    }
    if (const layout::HorizontalAlignment* ha = std::get_if<layout::HorizontalAlignment>(&var)) {
        switch (*ha) {
            case layout::HorizontalAlignment::Left:    return JsonValue("Left");
            case layout::HorizontalAlignment::Center:  return JsonValue("Center");
            case layout::HorizontalAlignment::Right:   return JsonValue("Right");
            case layout::HorizontalAlignment::Stretch: return JsonValue("Stretch");
        }
    }
    if (const layout::VerticalAlignment* va = std::get_if<layout::VerticalAlignment>(&var)) {
        switch (*va) {
            case layout::VerticalAlignment::Top:     return JsonValue("Top");
            case layout::VerticalAlignment::Center:  return JsonValue("Center");
            case layout::VerticalAlignment::Bottom:  return JsonValue("Bottom");
            case layout::VerticalAlignment::Stretch: return JsonValue("Stretch");
        }
    }
    if (const layout::Orientation* o = std::get_if<layout::Orientation>(&var)) {
        return JsonValue((*o == layout::Orientation::Horizontal) ? "Horizontal" : "Vertical");
    }
    if (const layout::Visibility* v = std::get_if<layout::Visibility>(&var)) {
        switch (*v) {
            case layout::Visibility::Visible:   return JsonValue("Visible");
            case layout::Visibility::Hidden:    return JsonValue("Hidden");
            case layout::Visibility::Collapsed: return JsonValue("Collapsed");
        }
    }
    if (const core::Color* c = std::get_if<core::Color>(&var)) {
        return JsonValue(ColorToHex(*c));
    }
    if (const BrushDescriptor* brush = std::get_if<BrushDescriptor>(&var)) {
        JsonValue::ObjectType obj;
        obj["type"] = (brush->type == BrushDescriptorType::Solid) ? JsonValue("Solid") : JsonValue("LinearGradient");
        obj["color"] = JsonValue(ColorToHex(brush->color));
        if (brush->type == BrushDescriptorType::LinearGradient) {
            obj["gradientEndColor"] = JsonValue(ColorToHex(brush->gradientEndColor));
        }
        return JsonValue(std::move(obj));
    }
    if (const text::FontDescriptor* font = std::get_if<text::FontDescriptor>(&var)) {
        JsonValue::ObjectType obj;
        obj["family"] = JsonValue(std::string(font->GetFamily()));
        obj["size"] = JsonValue(font->GetSize());
        return JsonValue(std::move(obj));
    }
    if (const ThemeTokenRef* tok = std::get_if<ThemeTokenRef>(&var)) {
        JsonValue::ObjectType obj;
        obj["$token"] = JsonValue(tok->tokenName);
        obj["$tokenType"] = (tok->tokenType == ThemeTokenType::Color) ? JsonValue("Color")
                          : (tok->tokenType == ThemeTokenType::Metric) ? JsonValue("Metric") : JsonValue("Typography");
        return JsonValue(std::move(obj));
    }

    return JsonValue(nullptr);
}

PropertyValue JsonToPropertyValue(const JsonValue& j) {
    if (j.IsBoolean()) {
        return PropertyValue(j.AsBool());
    }
    if (j.IsNumber()) {
        return PropertyValue(j.AsDouble());
    }
    if (j.IsString()) {
        const std::string& s = j.AsString();
        if (s.size() == 9 && s[0] == '#') {
            auto c = HexToColor(s);
            if (c.has_value()) return PropertyValue(*c);
        }
        if (s == "Left") return PropertyValue(layout::HorizontalAlignment::Left);
        if (s == "Center") return PropertyValue(layout::HorizontalAlignment::Center);
        if (s == "Right") return PropertyValue(layout::HorizontalAlignment::Right);
        if (s == "Stretch") return PropertyValue(layout::HorizontalAlignment::Stretch);
        if (s == "Top") return PropertyValue(layout::VerticalAlignment::Top);
        if (s == "Bottom") return PropertyValue(layout::VerticalAlignment::Bottom);
        if (s == "Horizontal") return PropertyValue(layout::Orientation::Horizontal);
        if (s == "Vertical") return PropertyValue(layout::Orientation::Vertical);
        if (s == "Visible") return PropertyValue(layout::Visibility::Visible);
        if (s == "Hidden") return PropertyValue(layout::Visibility::Hidden);
        if (s == "Collapsed") return PropertyValue(layout::Visibility::Collapsed);
        return PropertyValue(s);
    }
    if (j.IsObject()) {
        const auto& obj = j.AsObject();
        if (obj.count("$token") > 0) {
            ThemeTokenRef ref;
            ref.tokenName = obj.at("$token").AsString();
            if (obj.count("$tokenType") > 0) {
                const std::string& t = obj.at("$tokenType").AsString();
                if (t == "Metric") ref.tokenType = ThemeTokenType::Metric;
                else if (t == "Typography") ref.tokenType = ThemeTokenType::Typography;
                else ref.tokenType = ThemeTokenType::Color;
            }
            return PropertyValue(std::move(ref));
        }
        if (obj.count("x") > 0 && obj.count("y") > 0 && obj.count("width") > 0 && obj.count("height") > 0) {
            return PropertyValue(core::Rect(
                static_cast<float>(obj.at("x").AsDouble()),
                static_cast<float>(obj.at("y").AsDouble()),
                static_cast<float>(obj.at("width").AsDouble()),
                static_cast<float>(obj.at("height").AsDouble())
            ));
        }
        if (obj.count("x") > 0 && obj.count("y") > 0) {
            return PropertyValue(core::Point(
                static_cast<float>(obj.at("x").AsDouble()),
                static_cast<float>(obj.at("y").AsDouble())
            ));
        }
        if (obj.count("width") > 0 && obj.count("height") > 0) {
            return PropertyValue(core::Size(
                static_cast<float>(obj.at("width").AsDouble()),
                static_cast<float>(obj.at("height").AsDouble())
            ));
        }
        if (obj.count("left") > 0 && obj.count("top") > 0 && obj.count("right") > 0 && obj.count("bottom") > 0) {
            return PropertyValue(core::Thickness(
                static_cast<float>(obj.at("left").AsDouble()),
                static_cast<float>(obj.at("top").AsDouble()),
                static_cast<float>(obj.at("right").AsDouble()),
                static_cast<float>(obj.at("bottom").AsDouble())
            ));
        }
        if (obj.count("family") > 0 && obj.count("size") > 0) {
            return PropertyValue(text::FontDescriptor(
                obj.at("family").AsString(),
                static_cast<float>(obj.at("size").AsDouble())
            ));
        }
        if (obj.count("type") > 0 && obj.at("type").AsString() == "Solid" && obj.count("color") > 0) {
            BrushDescriptor b;
            b.type = BrushDescriptorType::Solid;
            auto c = HexToColor(obj.at("color").AsString());
            if (c.has_value()) b.color = *c;
            return PropertyValue(b);
        }
    }

    return PropertyValue{};
}

JsonValue SerializeNode(const WidgetNode* node) {
    if (!node) {
        return JsonValue(nullptr);
    }

    JsonValue::ObjectType nodeObj;
    nodeObj["id"] = JsonValue(node->GetName());
    nodeObj["uuid"] = JsonValue(node->GetUuid());
    nodeObj["type"] = JsonValue(std::string(NodeTypeName(node->GetType())));

    // 1. 自身屬性 (預設值剔除 Default Value Elision)
    JsonValue::ObjectType propsObj;
    for (const auto& kv : node->GetAllProperties()) {
        if (!PropertyMetadataRegistry::IsDefaultValue(node->GetType(), kv.first, kv.second)) {
            propsObj[kv.first] = PropertyValueToJson(kv.second);
        }
    }
    nodeObj["properties"] = JsonValue(std::move(propsObj));

    // 2. 附加屬性
    JsonValue::ObjectType attachedObj;
    for (const auto& kv : node->GetAllAttachedProperties()) {
        attachedObj[kv.first] = PropertyValueToJson(kv.second);
    }
    nodeObj["attachedProperties"] = JsonValue(std::move(attachedObj));

    // 3. 事件綁定
    JsonValue::ArrayType eventsArr;
    for (const auto& ev : node->GetEventBindings()) {
        JsonValue::ObjectType evObj;
        evObj["eventName"] = JsonValue(ev.eventName);
        evObj["handlerName"] = JsonValue(ev.handlerName);
        eventsArr.push_back(JsonValue(std::move(evObj)));
    }
    nodeObj["events"] = JsonValue(std::move(eventsArr));

    // 4. 資料綁定
    JsonValue::ArrayType bindingsArr;
    for (const auto& b : node->GetDataBindings()) {
        JsonValue::ObjectType bObj;
        bObj["mode"] = JsonValue((b.mode == BindingMode::OneWay) ? "OneWay" : "TwoWay");
        bObj["propertyName"] = JsonValue(b.propertyName);
        bObj["sourcePath"] = JsonValue(b.sourcePath);
        bindingsArr.push_back(JsonValue(std::move(bObj)));
    }
    nodeObj["bindings"] = JsonValue(std::move(bindingsArr));

    // 5. Grid 行列定義
    if (node->GetType() == NodeType::Grid) {
        JsonValue::ObjectType layoutDefObj;
        JsonValue::ArrayType rowsArr;
        for (const auto& row : node->GetRowDefinitions()) {
            JsonValue::ObjectType rowObj;
            rowObj["type"] = (row.height.type == layout::GridUnitType::Pixel) ? JsonValue("Pixel")
                           : (row.height.type == layout::GridUnitType::Auto)  ? JsonValue("Auto") : JsonValue("Star");
            rowObj["value"] = JsonValue(row.height.value);
            rowsArr.push_back(JsonValue(std::move(rowObj)));
        }
        layoutDefObj["rows"] = JsonValue(std::move(rowsArr));

        JsonValue::ArrayType colsArr;
        for (const auto& col : node->GetColumnDefinitions()) {
            JsonValue::ObjectType colObj;
            colObj["type"] = (col.width.type == layout::GridUnitType::Pixel) ? JsonValue("Pixel")
                           : (col.width.type == layout::GridUnitType::Auto)  ? JsonValue("Auto") : JsonValue("Star");
            colObj["value"] = JsonValue(col.width.value);
            colsArr.push_back(JsonValue(std::move(colObj)));
        }
        layoutDefObj["columns"] = JsonValue(std::move(colsArr));
        nodeObj["layoutDefinitions"] = JsonValue(std::move(layoutDefObj));
    }

    // 6. 遞迴子節點清單
    JsonValue::ArrayType childrenArr;
    for (const auto& child : node->GetChildren()) {
        childrenArr.push_back(SerializeNode(child.get()));
    }
    nodeObj["children"] = JsonValue(std::move(childrenArr));

    return JsonValue(std::move(nodeObj));
}

compat::expected<std::unique_ptr<WidgetNode>, DesignerError> DeserializeNode(const JsonValue& j, const std::string& path) {
    if (!j.IsObject()) {
        DesignerError err;
        err.code = DesignerErrorCode::InvalidJson;
        err.message = "Node at path must be an object";
        err.jsonPointer = path;
        return compat::unexpected(std::move(err));
    }

    const auto* typeVal = j.Find("type");
    if (!typeVal || !typeVal->IsString()) {
        DesignerError err;
        err.code = DesignerErrorCode::InvalidJson;
        err.message = "Node missing 'type' string property";
        err.jsonPointer = path + "/type";
        return compat::unexpected(std::move(err));
    }

    auto nodeTypeOpt = NodeTypeFromString(typeVal->AsString());
    if (!nodeTypeOpt.has_value()) {
        DesignerError err;
        err.code = DesignerErrorCode::InvalidJson;
        err.message = "Unknown node type: " + typeVal->AsString();
        err.jsonPointer = path + "/type";
        return compat::unexpected(std::move(err));
    }

    std::string idStr = "";
    if (const auto* idVal = j.Find("id")) {
        if (idVal->IsString()) idStr = idVal->AsString();
    }

    std::string uuidStr = "";
    if (const auto* uuidVal = j.Find("uuid")) {
        if (uuidVal->IsString()) uuidStr = uuidVal->AsString();
    }

    auto node = std::make_unique<WidgetNode>(nodeTypeOpt.value(), std::move(uuidStr), std::move(idStr));

    // 1. 自身屬性
    if (const auto* propsVal = j.Find("properties")) {
        if (propsVal->IsObject()) {
            for (const auto& kv : propsVal->AsObject()) {
                node->SetProperty(kv.first, JsonToPropertyValue(kv.second));
            }
        }
    }

    // 2. 附加屬性
    if (const auto* attVal = j.Find("attachedProperties")) {
        if (attVal->IsObject()) {
            for (const auto& kv : attVal->AsObject()) {
                node->SetAttachedProperty(kv.first, JsonToPropertyValue(kv.second));
            }
        }
    }

    // 3. 事件
    if (const auto* evArr = j.Find("events")) {
        if (evArr->IsArray()) {
            for (const auto& item : evArr->AsArray()) {
                if (item.IsObject()) {
                    EventBindingDescriptor ev;
                    if (const auto* en = item.Find("eventName")) ev.eventName = en->AsString();
                    if (const auto* hn = item.Find("handlerName")) ev.handlerName = hn->AsString();
                    node->AddEventBinding(std::move(ev));
                }
            }
        }
    }

    // 4. 綁定
    if (const auto* bArr = j.Find("bindings")) {
        if (bArr->IsArray()) {
            for (const auto& item : bArr->AsArray()) {
                if (item.IsObject()) {
                    DataBindingDescriptor bd;
                    if (const auto* pn = item.Find("propertyName")) bd.propertyName = pn->AsString();
                    if (const auto* sp = item.Find("sourcePath")) bd.sourcePath = sp->AsString();
                    if (const auto* md = item.Find("mode")) {
                        bd.mode = (md->AsString() == "TwoWay") ? BindingMode::TwoWay : BindingMode::OneWay;
                    }
                    node->AddDataBinding(std::move(bd));
                }
            }
        }
    }

    // 5. Grid 行列定義
    if (nodeTypeOpt.value() == NodeType::Grid) {
        if (const auto* ldef = j.Find("layoutDefinitions")) {
            if (ldef->IsObject()) {
                if (const auto* rows = ldef->Find("rows")) {
                    if (rows->IsArray()) {
                        std::vector<layout::RowDefinition> rowList;
                        for (const auto& r : rows->AsArray()) {
                            if (r.IsObject()) {
                                std::string t = r.Find("type") ? r.Find("type")->AsString() : "Star";
                                float val = r.Find("value") ? static_cast<float>(r.Find("value")->AsDouble()) : 1.0f;
                                auto unit = (t == "Pixel") ? layout::GridUnitType::Pixel
                                          : (t == "Auto")  ? layout::GridUnitType::Auto : layout::GridUnitType::Star;
                                rowList.emplace_back(layout::GridLength(val, unit));
                            }
                        }
                        node->SetRowDefinitions(std::move(rowList));
                    }
                }
                if (const auto* cols = ldef->Find("columns")) {
                    if (cols->IsArray()) {
                        std::vector<layout::ColumnDefinition> colList;
                        for (const auto& c : cols->AsArray()) {
                            if (c.IsObject()) {
                                std::string t = c.Find("type") ? c.Find("type")->AsString() : "Star";
                                float val = c.Find("value") ? static_cast<float>(c.Find("value")->AsDouble()) : 1.0f;
                                auto unit = (t == "Pixel") ? layout::GridUnitType::Pixel
                                          : (t == "Auto")  ? layout::GridUnitType::Auto : layout::GridUnitType::Star;
                                colList.emplace_back(layout::GridLength(val, unit));
                            }
                        }
                        node->SetColumnDefinitions(std::move(colList));
                    }
                }
            }
        }
    }

    // 6. 子節點
    if (const auto* chVal = j.Find("children")) {
        if (chVal->IsArray()) {
            const auto& arr = chVal->AsArray();
            for (size_t i = 0; i < arr.size(); ++i) {
                std::string childPath = path + "/children/" + std::to_string(i);
                auto childRes = DeserializeNode(arr[i], childPath);
                if (!childRes.has_value()) {
                    return compat::unexpected(childRes.error());
                }
                auto addRes = node->AddChild(std::move(childRes.value()));
                if (!addRes.has_value()) {
                    DesignerError err = addRes.error();
                    err.jsonPointer = childPath;
                    return compat::unexpected(std::move(err));
                }
            }
        }
    }

    return node;
}

} // anonymous namespace

compat::expected<std::string, DesignerError> FormSerializer::Serialize(const FormDocumentNode& doc) {
    JsonValue::ObjectType rootObj;
    rootObj["schemaVersion"] = JsonValue(CurrentSchemaVersion);

    // FormSettings
    const auto& s = doc.GetSettings();
    JsonValue::ObjectType settingsObj;
    settingsObj["className"] = JsonValue(s.className);
    settingsObj["targetNamespace"] = JsonValue(s.targetNamespace);
    settingsObj["title"] = JsonValue(s.title);
    settingsObj["defaultTheme"] = JsonValue(s.defaultTheme);
    settingsObj["useMvvm"] = JsonValue(s.useMvvm);
    settingsObj["useIoC"] = JsonValue(s.useIoC);

    JsonValue::ObjectType sizeObj;
    sizeObj["width"] = JsonValue(s.initialSize.width);
    sizeObj["height"] = JsonValue(s.initialSize.height);
    settingsObj["initialSize"] = JsonValue(std::move(sizeObj));

    rootObj["formSettings"] = JsonValue(std::move(settingsObj));

    // Root Widget
    if (doc.GetRoot()) {
        rootObj["root"] = SerializeNode(doc.GetRoot());
    } else {
        rootObj["root"] = JsonValue(nullptr);
    }

    JsonValue jsonRoot(std::move(rootObj));
    return jsonRoot.ToString(0);
}

compat::expected<std::unique_ptr<FormDocumentNode>, DesignerError> FormSerializer::Deserialize(compat::string_view jsonText) {
    auto jsonRes = JsonValue::Parse(jsonText);
    if (!jsonRes.has_value()) {
        return compat::unexpected(jsonRes.error());
    }

    const auto& rootVal = jsonRes.value();
    if (!rootVal.IsObject()) {
        DesignerError err;
        err.code = DesignerErrorCode::InvalidJson;
        err.message = "JSON root must be an object";
        err.jsonPointer = "/";
        return compat::unexpected(std::move(err));
    }

    // 檢查 schemaVersion
    const auto* verVal = rootVal.Find("schemaVersion");
    if (!verVal || !verVal->IsString()) {
        DesignerError err;
        err.code = DesignerErrorCode::SchemaMismatch;
        err.message = "Missing or invalid schemaVersion";
        err.jsonPointer = "/schemaVersion";
        return compat::unexpected(std::move(err));
    }

    FormSettings settings;
    if (const auto* setVal = rootVal.Find("formSettings")) {
        if (setVal->IsObject()) {
            if (const auto* cn = setVal->Find("className")) settings.className = cn->AsString();
            if (const auto* ns = setVal->Find("targetNamespace")) settings.targetNamespace = ns->AsString();
            if (const auto* tl = setVal->Find("title")) settings.title = tl->AsString();
            if (const auto* dt = setVal->Find("defaultTheme")) settings.defaultTheme = dt->AsString();
            if (const auto* mv = setVal->Find("useMvvm")) settings.useMvvm = mv->AsBool();
            if (const auto* ic = setVal->Find("useIoC")) settings.useIoC = ic->AsBool();
            if (const auto* sz = setVal->Find("initialSize")) {
                if (sz->IsObject()) {
                    if (const auto* w = sz->Find("width")) settings.initialSize.width = static_cast<float>(w->AsDouble());
                    if (const auto* h = sz->Find("height")) settings.initialSize.height = static_cast<float>(h->AsDouble());
                }
            }
        }
    }

    auto doc = std::make_unique<FormDocumentNode>(std::move(settings));

    if (const auto* rVal = rootVal.Find("root")) {
        if (!rVal->IsNull()) {
            auto nodeRes = DeserializeNode(*rVal, "/root");
            if (!nodeRes.has_value()) {
                return compat::unexpected(nodeRes.error());
            }
            doc->SetRoot(std::move(nodeRes.value()));
        }
    }

    return doc;
}

} // namespace designer
} // namespace cppui
