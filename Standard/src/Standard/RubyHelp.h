// Helper classes for writing Ruby extensions

#include "Standard/api.h"

namespace RubyHelp
{
	class Param
	{
	};

	typedef std::vector<Param> Params;

	template <class T>
	class TParam : public Param
	{
		typedef T Type;
	};

	class Int : public Param<int>
	{
	public:
		Type cpp(VALUE v) { return FIX2INT(v); }
		VALUE ruby(Type in) { return INT2FIX(in); }
	};
	
	class Double : public Param<double>
	{
	public:
		Type cpp(VALUE v) { return NUM2DBL(v); }
		VALUE ruby(Type in) { return DBL2NUM(in); }
	};

	template <class ClassType>
	class Class : public Param<ClassType*>
	{
	public:
		Type cpp(VALUE v) { return (Type)FIX2INT(v); }
		VALUE ruby(Type in) { return INT2FIX(in); }
	};

	class VABlock
	{
	public:
		VABlock()
		{
			va_start(vlist);
		}

		~VABlock()
		{
			va_end(vlist);
		}
		
		va_list vlist;
	};

	#define FUNC(x) static VALUE go(...) { VABlock block; x
	#define CPP(x) x().cpp(va_arg(vlist, VALUE))

	class Func
	{
	public:
		Func(const TDefine& owner, const char* name, int args)
		{
			rb_define_method(owner.rubyClass(), name, RUBY_METHOD_FUNC(go), initArgs);
		}

		FUNC(doThing)(
			CPP(Double),CPP(Int));
			return Qnil;
		}
	};

	template <class T, class Derived>
	class TDefine
	{
	public:
		TDefine(const char* name, int initArgs = 0, TDefine* extend = 0)
		{
			if (!extend)
				rubyClass = rb_define_class(name, rb_cObject);
			else
				rubyClass = rb_define_class(name, extend->rubyClass);

			rb_define_method(m_rubyClass, "initialize", RUBY_METHOD_FUNC(Derived::init), initArgs);
		}

		static void init()
		{
		}

		VALUE rubyClass() const
		{
			return m_rubyClass;
		}

	protected:
	    VALUE m_rubyClass;
	};

	template <class T>
	class TCPP
	{
		TCPP(VALUE self) :
			m_instance(*(T*)FIX2INT(rb_iv_get(self, "@cppobj")))
		{
		}

		TCPP(T& newInstance)
		{
			rb_iv_set(self, "@cppobj", INT2FIX(&newInstance));
		}

		T* operator -> () { return &m_instance; }

		static void define()
		{
		}

	protected:
		T& m_instance;
	};

	// C++ exceptions to ruby exceptions
	// From Paul Brannan comp.lang.ruby
	struct RubyException {
	VALUE ex;
	};

	#define RUBY_TRY \
	extern VALUE ruby_errinfo; \
	ruby_errinfo = Qnil; \
	try

	#define RUBY_CATCH \
	catch (RubyException & ex) { \
		rb_exc_raise(ex.ex); \
	} \
	catch (...) \
	{ \
		/* Can't raise the exception from here, because the C++ exception \
		* won't get properly destroyed. */ \
		ruby_errinfo = rb_exc_new2(rb_eRuntimeError, "Unknown error"); \
	} \
	if(!NIL_P(ruby_errinfo)) { \
		rb_exc_raise(ruby_errinfo); \
	} 

	// Example:
	// 
	//VALUE foo(VALUE /* self */, VALUE cb) {
	//RUBY_TRY
	//{
	//	struct Foo {
	//	Foo() { std::cout << "Foo" << std::endl; }
	//	~Foo() { std::cout << "~Foo" << std::endl; }
	//	} foo;
	//	RubyCallback rc(cb);
	//	rc.call();
	//}
	//RUBY_CATCH

	//}
};