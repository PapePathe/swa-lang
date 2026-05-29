#pragma once

#include <ast/visitor.h>
#include <cstddef>
#include <lexer/lexer.h>

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

#include <memory>
#include <string>

enum class TypeKind {
  Int,
  Float,
  Bool,
  Byte,
  String,
  Void,
  Struct,
  Pointer,
  Array,
  Slice,
  DirtyType
};

class Type {
public:
  Span span;
  explicit Type(Span s) : span(s) {}
  virtual ~Type() = default;
  virtual void Accept(ASTVisitor &v) = 0;
  virtual TypeKind GetKind() const = 0;
  virtual std::string GetName() const = 0;
  virtual bool IsEqual(const Type *other) const = 0;
  virtual std::unique_ptr<Type> Clone() const = 0;
};

class TypeInt : public Type {
public:
  explicit TypeInt(Span s) : Type(s) {}
  void Accept(ASTVisitor &v) override { v.Visit(this); };
  TypeKind GetKind() const override { return TypeKind::Int; }
  std::string GetName() const override { return "Int"; };
  bool IsEqual(const Type *other) const override {
    if (other->GetKind() != TypeKind::Int) {
      return false;
    }

    return true;
  }
  std::unique_ptr<Type> Clone() const override {
    return std::move(std::make_unique<TypeInt>(span));
  };
};

class TypeFloat : public Type {
public:
  explicit TypeFloat(Span s) : Type(s) {}
  void Accept(ASTVisitor &v) override { v.Visit(this); };
  TypeKind GetKind() const override { return TypeKind::Float; }
  std::string GetName() const override { return "Float"; };
  bool IsEqual(const Type *other) const override {
    if (other->GetKind() == GetKind()) {
      return true;
    }

    return false;
  }
  std::unique_ptr<Type> Clone() const override {
    return std::move(std::make_unique<TypeFloat>(span));
  };
};

class TypeBool : public Type {
public:
  explicit TypeBool(Span s) : Type(s) {}
  void Accept(ASTVisitor &v) override { v.Visit(this); };
  TypeKind GetKind() const override { return TypeKind::Bool; }
  std::string GetName() const override { return "Bool"; };
  bool IsEqual(const Type *other) const override {
    if (GetKind() == other->GetKind()) {
      return true;
    }
    return false;
  }
  std::unique_ptr<Type> Clone() const override {
    return std::move(std::make_unique<TypeBool>(span));
  };
};

class TypeByte : public Type {

public:
  explicit TypeByte(Span s) : Type(s) {}
  void Accept(ASTVisitor &v) override { v.Visit(this); };
  TypeKind GetKind() const override { return TypeKind::Byte; }
  std::string GetName() const override { return "Byte"; };
  bool IsEqual(const Type *other) const override {
    if (GetKind() == other->GetKind()) {
      return true;
    }
    return false;
  }
  std::unique_ptr<Type> Clone() const override {
    return std::move(std::make_unique<TypeByte>(span));
  };
};

class TypeString : public Type {
public:
  explicit TypeString(Span s) : Type(s) {}
  void Accept(ASTVisitor &v) override { v.Visit(this); };
  TypeKind GetKind() const override { return TypeKind::String; }
  std::string GetName() const override { return "String"; };
  bool IsEqual(const Type *other) const override {
    if (GetKind() == other->GetKind()) {
      return true;
    }
    return false;
  }

  std::unique_ptr<Type> Clone() const override {
    return std::move(std::make_unique<TypeString>(span));
  };
};

class TypeVoid : public Type {
public:
  explicit TypeVoid(Span s) : Type(s) {}
  void Accept(ASTVisitor &v) override { v.Visit(this); };
  TypeKind GetKind() const override { return TypeKind::Void; }
  std::string GetName() const override { return "Void"; };
  bool IsEqual(const Type *other) const override {
    if (GetKind() == other->GetKind()) {
      return true;
    }
    return false;
  }

  std::unique_ptr<Type> Clone() const override {
    return std::move(std::make_unique<TypeVoid>(span));
  };
};

class TypeDirty : public Type {
public:
  explicit TypeDirty(Span s) : Type(s) {}
  void Accept(ASTVisitor &v) override {
    throw std::runtime_error("why would you visit type dirty");
  };
  TypeKind GetKind() const override { return TypeKind::DirtyType; }
  std::string GetName() const override { return "Dirty(TypeChecking failed)"; };
  bool IsEqual(const Type *other) const override {
    if (GetKind() == other->GetKind()) {
      return true;
    }
    return false;
  }

  std::unique_ptr<Type> Clone() const override {
    return std::move(std::make_unique<TypeDirty>(span));
  };
};

class TypeStruct : public Type {
public:
  std::string Name;
  TypeStruct(std::string name, Span s) : Type(s), Name(name) {}
  void Accept(ASTVisitor &v) override { v.Visit(this); };
  TypeKind GetKind() const override { return TypeKind::Struct; }
  std::string GetName() const override { return "Struct"; };
  bool IsEqual(const Type *other) const override {
    if (GetKind() == other->GetKind()) {
      return true;
    }
    return false;
  }

  std::unique_ptr<Type> Clone() const override {
    return std::move(std::make_unique<TypeStruct>(Name, span));
  };
};

class TypePointer : public Type {
public:
  std::unique_ptr<Type> T;
  TypePointer(std::unique_ptr<Type> t, Span s) : Type(s), T(std::move(t)) {}
  void Accept(ASTVisitor &v) override { v.Visit(this); };
  TypeKind GetKind() const override { return TypeKind::Pointer; }
  std::string GetName() const override { return "Pointer"; };
  bool IsEqual(const Type *other) const override {
    if (GetKind() == other->GetKind()) {
      return true;
    }
    return false;
  }

  std::unique_ptr<Type> Clone() const override {
    return std::move(std::make_unique<TypePointer>(T->Clone(), span));
  };
};

class TypeArray : public Type {
public:
  size_t Size;
  std::unique_ptr<Type> T;
  TypeArray(size_t size, std::unique_ptr<Type> t, Span s)
      : Size(size), Type(s), T(std::move(t)) {}
  void Accept(ASTVisitor &v) override { v.Visit(this); };
  TypeKind GetKind() const override { return TypeKind::Array; }
  std::string GetName() const override {
    return "Array[" + std::to_string(Size) + " * " + T->GetName() + "]";
  };
  bool IsEqual(const Type *other) const override {
    if (GetKind() != other->GetKind()) {
      return false;
    }

    const auto *otherArray = static_cast<const TypeArray *>(other);

    if (Size != otherArray->Size) {
      return false;
    }

    return T->IsEqual(otherArray->T.get());
  }

  std::unique_ptr<Type> Clone() const override {
    return std::move(std::make_unique<TypeArray>(Size, T->Clone(), span));
  };
};

class TypeSlice : public Type {
public:
  std::unique_ptr<Type> T;
  TypeSlice(std::unique_ptr<Type> t, Span s) : Type(s), T(std::move(t)) {}
  void Accept(ASTVisitor &v) override { v.Visit(this); };
  TypeKind GetKind() const override { return TypeKind::Slice; }
  std::string GetName() const override { return "Slice"; };
  bool IsEqual(const Type *other) const override {
    if (GetKind() == other->GetKind()) {
      return true;
    }
    return false;
  }
  std::unique_ptr<Type> Clone() const override {
    return std::move(std::make_unique<TypePointer>(T->Clone(), span));
  };
};
