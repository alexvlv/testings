#include "Cam.h"
#include <QDebug>

//-------------------------------------------------------------------------
template<typename Enum>
inline QDebug operator<<(QDebug dbg, Enum e)
{
	QDebugStateSaver saver(dbg);

	static_assert(
		std::is_same_v<Enum, Cam::Param> ||
		std::is_same_v<Enum, Cam::Command> ||
		std::is_same_v<Enum, Cam::Arg>,
		"operator<< only supports Cam enums"
	);

	dbg.nospace() << Cam::toString(e);
	return dbg;
}
//-------------------------------------------------------------------------
Cam::Cam(uint idx)
	:idx(idx)
{
	Cam::Param p = Cam::Param::HOST;
	Cam::Command c = Cam::Command::NIGHT_ON;
	Cam::Arg a = Cam::Arg::TYPE;

	qDebug() << p << c << a;

	QDebug dbg = qDebug();//.nospace().noquote();
	dbg << Qt::endl;
	for (Param c : ParamIterable) {
		dbg << c;
	}
	dbg << Qt::endl;
	for (Command c : CommandIterable) {
		dbg << c;
	}
	dbg << Qt::endl;
	for (Arg c : ArgIterable) {
		dbg << c;
	}
	dbg << Qt::endl;
}
//-------------------------------------------------------------------------
void Cam::update(const IniStreamConfig &cfg)
{
	Q_UNUSED(cfg);
	//const QString sname = QStringLiteral("cam%1").arg(idx+1);
	//if(!cfg.hasSection(sname)) return;
/*
	for (Param prm : ParamIterable) {
		QString sprm = cfg.get(sname,toString(prm)).toString();
		if(sprm != params[prm]) {
			qDebug().noquote() << sname << prm << "[" << params[prm] << "] => [" << sprm << "]";
			params[prm] = sprm;
		}
	}
*/
}
//-------------------------------------------------------------------------
#if 0
	using U = std::underlying_type_t<Command>;
	for (int i = 0; i < static_cast<U>(Command::Count); ++i) {
		auto c = static_cast<Command>(i);
		commands[c] = cfg.get(sname,toString(c)).toString();
		qDebug() << sname << c << commands[c];
	}
#endif
