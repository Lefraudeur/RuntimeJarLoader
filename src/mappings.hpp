#pragma once

#include "meta_jni.hpp"
#include<string>

namespace maps
{
	BEGIN_KLASS_DEF(Object, "java/lang/Object")
	END_KLASS_DEF()

	BEGIN_KLASS_DEF(Class, "java/lang/Class")
		operator jclass() const
		{
			return (jclass)object_instance;
		}
	END_KLASS_DEF()

	BEGIN_KLASS_DEF(String, "java/lang/String")
		static String create(const char* str)
		{
			return String(jni::get_env()->NewStringUTF(str));
		}


		std::string to_string()
		{
			if (!object_instance) return std::string();
			jstring str_obj = (jstring)object_instance;
			jsize utf8_size = jni::get_env()->GetStringUTFLength(str_obj);
			jsize size = jni::get_env()->GetStringLength(str_obj);

			std::string str(utf8_size, '\0');
			jni::get_env()->GetStringUTFRegion(str_obj, 0, size, str.data());
			return str;
		}
	END_KLASS_DEF()

	BEGIN_KLASS_DEF(Collection, "java/util/Collection")
		jni::method<jni::array<Object>, "toArray"> toArray{ *this };
	END_KLASS_DEF()
	BEGIN_KLASS_DEF_EX(List, "java/util/List", Collection)
	END_KLASS_DEF()

	BEGIN_KLASS_DEF(URL, "java/net/URL")
		jni::constructor<String> constructor{ *this };

		jni::method<String, "toString"> toString{ *this };
	END_KLASS_DEF()

	BEGIN_KLASS_DEF(URLClassLoader, "java/net/URLClassLoader");
		jni::constructor<jni::array<URL>> constructor{ *this };

		jni::method<void, "addURL", jni::NOT_STATIC, URL> addURL{ *this };
		jni::method<Class, "findClass", jni::NOT_STATIC, String> findClass{ *this };
		jni::method<jni::array<URL>, "getURLs", jni::NOT_STATIC> getURLs{ *this };
	END_KLASS_DEF()

	BEGIN_KLASS_DEF(Main, "io/github/lefraudeur/Main");
		jni::method<void, "main", jni::STATIC, jni::array<String>> main{ *this };
	END_KLASS_DEF()
}