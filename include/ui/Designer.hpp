/**
 * @file Designer.hpp
 * @brief CPPUI Visual Form Designer 視覺化表單生成器頂層總括標頭檔 (Master Umbrella Header)
 * 使用者引入本標頭檔即可使用 Designer 提供的所有 AST 領域模型、屬性元資料、Undo/Redo 狀態機與序列化功能。
 */

#pragma once

#include "compat/Compat.hpp"
#include "ui/designer/ast/AstTypes.hpp"
#include "ui/designer/ast/WidgetNode.hpp"
#include "ui/designer/ast/FormDocumentNode.hpp"
#include "ui/designer/ast/CardinalityValidator.hpp"
#include "ui/designer/ast/IdentifierValidator.hpp"
#include "ui/designer/metadata/PropertyValue.hpp"
#include "ui/designer/metadata/PropertyDescriptor.hpp"
#include "ui/designer/metadata/AttachedProperties.hpp"
#include "ui/designer/commands/IUndoableCommand.hpp"
#include "ui/designer/commands/WidgetCommands.hpp"
#include "ui/designer/commands/PropertyCommands.hpp"
#include "ui/designer/commands/MacroCommand.hpp"
#include "ui/designer/commands/CommandHistoryManager.hpp"
#include "ui/designer/serialization/SchemaVersion.hpp"
#include "ui/designer/serialization/FormSerializer.hpp"
#include "ui/designer/codegen/CodegenTypes.hpp"
#include "ui/designer/codegen/FluentDslEmitter.hpp"
#include "ui/designer/codegen/FormClassEmitter.hpp"
#include "ui/designer/codegen/CMakeSnippetEmitter.hpp"
#include "ui/designer/codegen/FormCodegenPipeline.hpp"
#include "ui/designer/surface/SurfaceTypes.hpp"
#include "ui/designer/surface/SelectionModel.hpp"
#include "ui/designer/surface/DesignerHitTest.hpp"
#include "ui/designer/surface/SnapEngine.hpp"
#include "ui/designer/surface/AdornerOverlay.hpp"
#include "ui/designer/surface/DesignSurface.hpp"
#include "ui/designer/inspector/InspectorTypes.hpp"
#include "ui/designer/inspector/InheritanceMetadataRegistry.hpp"
#include "ui/designer/inspector/PropertyEditors.hpp"
#include "ui/designer/inspector/InspectorPanel.hpp"
#include "ui/designer/preview/LiveCodePreviewPanel.hpp"
#include "ui/designer/preview/LiveFormRunner.hpp"
#include "ui/designer/exporter/ProjectExporter.hpp"
