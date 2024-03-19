//===- BuildInitializers.cpp ----------------------------------------------===//
//
//                             The ONNC Project
//
// See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include <onnc/Transforms/BuildInitializers.h>
#include <onnc/Core/PassSupport.h>
#include <onnc/IR/Compute/Initializer.h>
#include <onnc/IR/IRBuilder.h>
#include <onnc/Config/ONNX.h>

using namespace onnc;

//===----------------------------------------------------------------------===//
// BuildInitializers
//===----------------------------------------------------------------------===//
Pass::ReturnType
BuildInitializers::runOnGraphs(xGraph& pTG, ComputeGraph& pCG)
{
  // Create initializer. In ONNC, initializer is a kind of ComputeOperator.
  // XXX: ONNX doesn't define new types for these data structures
  std::vector<xTensor>::const_iterator tensor, tEnd =
      pTG.initializers().end();

  std::vector<std::string>::const_iterator tname =
      pTG.initializer_names().begin();

  tensor = pTG.initializers().begin();

  std::map<std::string, const xTensor*> initializers;
  while (tensor != tEnd) {
    initializers[*tname] = &*tensor;
    ++tname;
    ++tensor;
  }

  xGraphNodeListIterator tg_node, tg_end = pTG.end();
  for (tg_node = pTG.begin(); tg_node != tg_end; ++tg_node) {
   for (xValue* v : tg_node->inputs()) {
      auto it = initializers.find(v->uniqueName());
      if (initializers.end() != it) {
         // The value appears in an initializer, we should create corresponding
         // initializer to handle with the value.
         Initializer* init = pCG.addOperator<onnc::Initializer>(v->uniqueName());
         Tensor* value = IRBuilder::CreateComputeTensor(pCG, *v, *it->second);
         init->setTensor(*value);
      }
    }
  } // end of trip on all input values

  return Pass::kModuleChanged;
}

//===----------------------------------------------------------------------===//
// Non-member functions
//===----------------------------------------------------------------------===//
INITIALIZE_PASS(BuildInitializers, "BuildInitializers");

ModulePass *onnc::CreateBuildInitializers()
{
  return new BuildInitializers();
}
