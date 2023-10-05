//
// Copyright 2019 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#ifndef ZETASQL_TESTDATA_SAMPLE_CATALOG_H_
#define ZETASQL_TESTDATA_SAMPLE_CATALOG_H_

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "google/protobuf/descriptor.h"
#include "google/protobuf/descriptor_database.h"
#include "zetasql/public/analyzer_output.h"
#include "zetasql/public/function.h"
#include "zetasql/public/language_options.h"
#include "zetasql/public/simple_catalog.h"
#include "zetasql/public/type.h"
#include "absl/container/node_hash_map.h"
#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"

namespace zetasql {

// SampleCatalog provides a SimpleCatalog loaded with a shared sample schema,
// used by several tests.  Look at the .cc file to see what's in the Catalog.
// All proto types compiled into this binary will be available in the catalog.
class SampleCatalog {
 public:
  // Default constructor using default LanguageOptions and a locally owned
  // TypeFactory.
  SampleCatalog();
  // Constructor given 'language_options' and an optional 'type_factory'.
  // If 'type_factory' is specified then it must outlive this SampleCatalog
  // and this SampleCatalog does not take ownership of it.  If 'type_factory'
  // is not specified then a locally owned TypeFactory is created and
  // used instead.
  explicit SampleCatalog(const LanguageOptions& language_options,
                         TypeFactory* type_factory = nullptr);

  // Constructor given 'builtin_function_options' and optional 'type_factory'.
  // If 'type_factory' is specified then it must outlive this SampleCatalog
  // and this SampleCatalog does not take ownership of it.  If 'type_factory'
  // is not specified then a locally owned TypeFactory is created and
  // used instead.
  explicit SampleCatalog(
      const ZetaSQLBuiltinFunctionOptions& builtin_function_options,
      TypeFactory* type_factory = nullptr);

  SampleCatalog(const SampleCatalog&) = delete;
  SampleCatalog& operator=(const SampleCatalog&) = delete;
  ~SampleCatalog();

  SimpleCatalog* catalog() { return catalog_.get(); }

  TypeFactory* type_factory() { return types_; }

  // Useful for configuring EvaluatorTableIterators for tables in the catalog.
  SimpleTable* GetTableOrDie(absl::string_view name);
  absl::StatusOr<SimpleTable*> GetTable(absl::string_view name);

 private:
  std::unique_ptr<google::protobuf::DescriptorPoolDatabase> alt_descriptor_database_;
  std::unique_ptr<google::protobuf::DescriptorPool> alt_descriptor_pool_;
  std::unique_ptr<google::protobuf::DescriptorPool> ambiguous_has_descriptor_pool_;
  std::unique_ptr<TypeFactory> internal_type_factory_;
  std::unique_ptr<SimpleCatalog> catalog_;
  TypeFactory* types_;  // Not owned.

  void LoadCatalog(const LanguageOptions& language_options);
  void LoadCatalogBuiltins(const LanguageOptions& language_options);
  void LoadCatalogBuiltins(
      const ZetaSQLBuiltinFunctionOptions& builtin_function_options);
  void LoadCatalogImpl(const LanguageOptions& language_options);
  void LoadTypes();
  void LoadTables();
  void LoadProtoTables();
  void LoadViews(const LanguageOptions& language_options);
  void LoadNestedCatalogs();
  void AddFunctionWithArgumentType(std::string type_name, const Type* arg_type);

  // Creates and adds the Function to the catalog.
  // This performs some basic validation.
  // The group used is 'sample_functions'.
  const Function* AddFunction(
      absl::string_view name, Function::Mode mode,
      std::vector<FunctionSignature> function_signatures,
      FunctionOptions function_options = {});

  void LoadFunctionsWithStructArgs();
  void LoadFunctions();
  // Similar to `LoadFunctions`, loads test functions into the sample catalog.
  // We have to split the original `LoadFunctions` function into two otherwise
  // the stack size limit is reached.
  void LoadFunctions2();
  void LoadExtendedSubscriptFunctions();
  void LoadFunctionsWithDefaultArguments();
  void LoadTemplatedSQLUDFs();

  // Loads several table-valued functions into the sample catalog. For a full
  // list of the signatures added, please see the beginning of the method
  // definition. LoadTableValuedFunctions() has gotten so large that we have to
  // split it up in order to avoid lint warnings.
  void LoadTableValuedFunctions1();
  void LoadTableValuedFunctions2();
  void LoadTVFWithExtraColumns();
  void LoadConnectionTableValuedFunctions();
  void LoadDescriptorTableValuedFunctions();
  void LoadTableValuedFunctionsWithDeprecationWarnings();

  // Add a SQL table function to catalog starting from a full create table
  // function statement.
  void AddSqlDefinedTableFunctionFromCreate(
      absl::string_view create_table_function,
      const LanguageOptions& language_options,
      absl::string_view user_id_column = "");
  void LoadNonTemplatedSqlTableValuedFunctions(
      const LanguageOptions& language_options);
  void LoadTemplatedSQLTableValuedFunctions();
  void LoadTableValuedFunctionsWithAnonymizationUid();

  void AddProcedureWithArgumentType(std::string type_name,
                                    const Type* arg_type);
  void LoadProcedures();
  void LoadConstants();
  void LoadConnections();
  void LoadSequences();
  // Load signatures for well known functional programming functions for example
  // FILTER, TRANSFORM, REDUCE.
  void LoadWellKnownLambdaArgFunctions();
  // Contrived signatures are loaded in order to demonstrate the behavior of
  // lambda signature matching and resolving for unusual cases.
  // This include:
  //  * Using lambda with repeated arguments.
  //  * Using lambda with named arguments.
  //  * Possible signatures that could result in type inference failure for
  //  various combinations of templated lambda arguments and other arguments.
  void LoadContrivedLambdaArgFunctions();

  void AddOwnedTable(SimpleTable* table);
  void AddGeneratedColumnToTable(std::string column_name,
                                 std::vector<std::string> expression_columns,
                                 std::string generated_expr,
                                 SimpleTable* table);

  // Add a SQLFunction to catalog_ with a SQL expression as the function body.
  void AddSqlDefinedFunction(absl::string_view name,
                             FunctionSignature signature,
                             const std::vector<std::string>& argument_names,
                             absl::string_view function_body_sql,
                             const LanguageOptions& language_options);
  // Add a SQL function to catalog starting from a full create_function
  // statement.
  void AddSqlDefinedFunctionFromCreate(
      absl::string_view create_function,
      const LanguageOptions& language_options, bool inline_sql_functions = true,
      std::optional<FunctionOptions> function_options = std::nullopt);

  void LoadSqlFunctions(const LanguageOptions& language_options);

  // Helpers for LoadSqlFunctions so that its both logically broken up and
  // so that its less troublesome for dbg build stacks.
  void LoadScalarSqlFunctions(const LanguageOptions& language_options);
  void LoadScalarSqlFunctionsFromStandardModule(
      const LanguageOptions& language_options);
  void LoadDeepScalarSqlFunctions(const LanguageOptions& language_options);
  void LoadScalarSqlFunctionTemplates(const LanguageOptions& language_options);
  void LoadAggregateSqlFunctions(const LanguageOptions& language_options);

  // This can be used force linking of a proto for the generated_pool.
  // This may be required if a proto is referenced in file-based tests
  // (such as analyzer test), but not otherwise directly linked.
  // We don't force linking the entire test_schema since we may need
  // to test this partial-linkage in other contexts (and it's expensive).
  // Note, this function isn't actually called. But it _does_ need to be
  // defined in the class to ensure it can't be pruned.
  // This is a all weird linker magic.
  void ForceLinkProtoTypes();

  const ProtoType* GetProtoType(const google::protobuf::Descriptor* descriptor);
  const EnumType* GetEnumType(const google::protobuf::EnumDescriptor* descriptor);

  const ArrayType* int32array_type_;
  const ArrayType* int64array_type_;
  const ArrayType* uint32array_type_;
  const ArrayType* uint64array_type_;
  const ArrayType* bytes_array_type_;
  const ArrayType* bool_array_type_;
  const ArrayType* float_array_type_;
  const ArrayType* double_array_type_;
  const ArrayType* date_array_type_;
  const ArrayType* string_array_type_;
  const ArrayType* timestamp_array_type_;
  const ArrayType* proto_array_type_;
  const ArrayType* struct_array_type_;
  const ArrayType* json_array_type_;

  const EnumType* enum_TestEnum_;
  const EnumType* enum_AnotherTestEnum_;
  const EnumType* enum_TestEnumWithAnnotations_;
  const ProtoType* proto_KitchenSinkPB_;
  const ProtoType* proto_MessageWithKitchenSinkPB_;
  const ProtoType* proto_CivilTimeTypesSinkPB_;
  const ProtoType* proto_TestExtraPB_;
  const ProtoType* proto_abPB_;
  const ProtoType* proto_bcPB_;

  const ProtoType* proto_EmptyMessage_;
  const ProtoType* proto3_KitchenSinkPB_;
  const ProtoType* proto3_MessageWithInvalidMap_;
  const ProtoType* proto_ambiguous_has_;
  const ProtoType* proto_field_formats_proto_;
  const ProtoType* proto_MessageWithMapField_;

  // STRUCT<a INT32, b STRING>
  const StructType* struct_type_;
  // STRUCT<c INT32, d STRUCT<a INT32, b STRING>>
  const StructType* nested_struct_type_;
  // STRUCT<e INT32, f STRUCT<c INT32, d STRUCT<a INT32, b STRING>>>
  const StructType* doubly_nested_struct_type_;
  // STRUCT<x INT64, y STRUCT<a INT32, b STRING>,
  //        z ARRAY<STRUCT<a INT32, b STRING>>>
  const StructType* struct_with_array_field_type_;
  // STRUCT<x INT64>
  const StructType* struct_with_one_field_type_;
  // STRUCT<kitchen_sink KitchenSinkPB, s STRUCT<kitchen_sink KitchenSinkPB>>
  const StructType* struct_with_kitchen_sink_type_;
  // STRUCT<a INT64, b ARRAY<STRUCT<kitchen_sink KitchenSinkPB>>>
  const StructType* struct_of_array_of_struct_with_kitchen_sink_type_;

  const SimpleTable* key_value_table_;

  // A constant to load. Owned by this catalog to get coverage for
  // SimpleCatalog::AddConstant().
  std::unique_ptr<SimpleConstant> owned_constant_;

  // Pointers are owned by 'catalog_'.
  absl::node_hash_map<std::string, SimpleTable*> tables_;

  // Connections owned by this catalog.
  std::unordered_map<std::string, std::unique_ptr<SimpleConnection>>
      owned_connections_;

  // Sequences owned by this catalog.
  std::unordered_map<std::string, std::unique_ptr<SimpleSequence>>
      owned_sequences_;

  // Manages the lifetime of ResolvedAST objects for SQL defined statements like
  // views, SQL functions, column expressions, or SQL TVFs.
  std::vector<std::unique_ptr<const AnalyzerOutput>> sql_object_artifacts_;
};

}  // namespace zetasql

#endif  // ZETASQL_TESTDATA_SAMPLE_CATALOG_H_
