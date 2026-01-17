#ifndef CAM_H
#define CAM_H

// X-macro list: single source for enums
#define CAM_PARAMS_LIST(Enum) \
	CAM_X_HELPER(Enum, URL_H) \
	CAM_X_HELPER(Enum, URL_L) \
	CAM_X_HELPER(Enum, HOST) \
	CAM_X_HELPER(Enum, LOGIN) \
	CAM_X_HELPER(Enum, PASSWORD) \
	CAM_X_HELPER(Enum, SSH_PORT) \
	CAM_X_HELPER(Enum, HTTP_LOGIN) \
	CAM_X_HELPER(Enum, HTTP_PASSWORD) \
	CAM_X_HELPER(Enum, HTTP_PORT)

#define CAM_COMMANDS_LIST(Enum) \
	CAM_X_HELPER(Enum, NIGHT_ON) \
	CAM_X_HELPER(Enum, NIGHT_OFF) \
	CAM_X_HELPER(Enum, BITRATE1) \
	CAM_X_HELPER(Enum, BITRATE2)

#define CAM_ARG_LIST(Enum) \
	CAM_X_HELPER(Enum, TYPE) \
	CAM_X_HELPER(Enum, EXECUTABLE) \
	CAM_X_HELPER(Enum, ARG) \
	CAM_X_HELPER(Enum, METHOD) \
	CAM_X_HELPER(Enum, PATH)

#define DECL_ENUM(EnumName, LIST) \
	enum class EnumName : int { \
		LIST \
		Count \
	};

#define DECL_ENUM_ITERATOR(Enum, LIST) \
	inline static constexpr Enum Enum##Iterable[] = { LIST(Enum) };

#include <QMap>
#include <QStringView>

class IniStreamConfig;

class Cam
{
public:
	Cam(uint idx);
	void update(const IniStreamConfig &);

private:
	uint idx = 0;

#define CAM_X_HELPER(Enum, name) name,
	DECL_ENUM(Param,   CAM_PARAMS_LIST(Param))
	DECL_ENUM(Command, CAM_COMMANDS_LIST(Command))
	DECL_ENUM(Arg,     CAM_ARG_LIST(Arg))
#undef CAM_X_HELPER

#define CAM_X_HELPER(Enum, name) QStringLiteral(#name),
	inline static const std::array<QStringView, static_cast<int>(Param::Count)> ParamStrings = { CAM_PARAMS_LIST(Param) };
	inline static const std::array<QStringView, static_cast<int>(Command::Count)> CommandStrings = { CAM_COMMANDS_LIST(Command) };
	inline static const std::array<QStringView, static_cast<int>(Arg::Count)> ArgStrings = { CAM_ARG_LIST(Arg) };
#undef CAM_X_HELPER

#define CAM_X_HELPER(Enum, name) Enum::name,
	DECL_ENUM_ITERATOR(Param,   CAM_PARAMS_LIST)
	DECL_ENUM_ITERATOR(Command, CAM_COMMANDS_LIST)
	DECL_ENUM_ITERATOR(Arg,     CAM_ARG_LIST)
#undef CAM_X_HELPER

	template<typename Enum, typename Array>
	static QStringView enumToString(Enum value, const Array &strings)
	{
		static_assert(std::is_enum<Enum>::value, "Enum required");

		const int idx = static_cast<int>(value);
		if (idx < 0 || idx >= static_cast<int>(strings.size()))
			return QStringLiteral("Unknown");

		return strings[idx];
	}

	template<typename Enum, typename Array>
	static QString toQString(Enum e, const Array &strings)
	{
		QStringView sv = enumToString(e, strings);
		return QString(sv.data(), sv.size());
	}

	static QString toString(Param p)    { return toQString(p, ParamStrings); }
	static QString toString(Command c)  { return toQString(c, CommandStrings); }
	static QString toString(Arg a)      { return toQString(a, ArgStrings); }

	template<typename Enum>
	friend inline QDebug operator<<(QDebug dbg, Enum e);

	QMap<Param, QString> params;
};

#undef CAM_PARAMS_LIST
#endif // CAM_H
