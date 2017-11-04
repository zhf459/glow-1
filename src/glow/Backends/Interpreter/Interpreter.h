#ifndef GLOW_INTERPRETER_INTERPRETER_H
#define GLOW_INTERPRETER_INTERPRETER_H

#include "glow/Backends/Backend.h"
#include "glow/Base/Tensor.h"

#include "llvm/ADT/ArrayRef.h"

#include <unordered_map>

namespace glow {

class Context;
class Module;
class Value;
class Tensor;
class Variable;

// Forward declare all of the classes.
#define DEF_VALUE(CLASS, NAME) class CLASS;
#define DEF_INSTR(CLASS, NAME) class CLASS;
#include "AutoGenInstr.def"

/// This is the IR-interpreter. It owns the IR, and the heap, and is able to
/// execute the instructions one at a time.
class Interpreter final : public Backend {
  /// The Module that holds the IR. This does not own the module.
  Module *M_;
  /// Maps values to Tensors, that are owned by this class.
  std::unordered_map<const Value *, Tensor *> tensors_;

  /// Maps values to Tensors, that are *not* owned by this class.
  std::unordered_map<const Value *, Tensor *> externalTensors_;

public:
  /// Ctor.
  explicit Interpreter(Module *M) : M_(M) {}

  /// @name Backend methods.
  /// This is the implementation of the Backend interface.
  ///@{
  ~Interpreter() override;

  void clear() override;

  void init() override;

  void doForwardPass(bool isTrain) override;

  void registerGraphTensor(const Value *v, Tensor *t) override;

  Tensor *getTensor(const Variable *v) const override;

  Tensor *getGradTensor(const Variable *v) const override;
  /// @}

private:
  /// \returns a pointer to the tensor that is saved under \p v.
  Tensor *getTensor(const Value *v) const;

  /// Allocate a tensor to back the value \p v. Do not allocate anything if a
  /// tensor is already allocated for \p v.
  /// \returns a tensor for \p v.
  Tensor *getOrCreateTensor(const Value *v);

  /// If a tensor is allocated for \p v then delete it.
  void deleteTensor(const Value *v);

  /// \returns a float-handle to the tensor that is stored at \p v.
  Handle<float> getWeightHandle(Value *v) const;

  /// \returns a float-handle to the gradient tensor that matches \p v. Notice
  /// that this API is only valid when the module is compiled in training mode.
  Handle<float> getGradHandle(Value *v) const;

  /// \returns a pointer to the gradient tensor that matches \p v. Notice
  /// that this API is only valid when the module is compiled in training mode.
  Tensor *getGradTensor(const Value *v) const;

  /// \returns True if the value \p has an associated gradient tensor.
  bool hasGradTensor(const Value *v) const;

  /// @name Interpreter methods. This is a list of method declerations that are
  /// used by the interpreter to dispatch different instructions.
  ///@{

#define DEF_VALUE(CLASS, NAME)
#define DEF_INSTR(CLASS, NAME) void fwd##CLASS(bool isTrain, const CLASS *I);
#include "AutoGenInstr.def"

  void fwdBatchNormalizationInst_infer(const BatchNormalizationInst *I);
  void fwdBatchNormalizationInst_train(const BatchNormalizationInst *I);
  ///@}
};

/// Create a new instance of the Interpreter backend.
inline Backend *createInterpreter(Module *M) { return new Interpreter(M); }

} // namespace glow

#endif // GLOW_INTERPRETER_INTERPRETER_H