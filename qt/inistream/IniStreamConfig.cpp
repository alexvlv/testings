#include "IniStreamConfig.h"
#include <QDebug>

void IniStreamConfig::parse(const QByteArray &chunk)
{
	QString data = QString::fromUtf8(chunk);
	QString currentSection;

	for (const QString &line : data.split('\n', Qt::SkipEmptyParts)) {
		QString s = line.trimmed();

		if (s.isEmpty() || s.startsWith(';') || s.startsWith('#'))
			continue; // skip empty lines and comments

		// Section header
		if (s.startsWith('[') && s.endsWith(']')) {
			currentSection = normalize(s.mid(1, s.size() - 2).trimmed());
			continue;
		}

		int eq = s.indexOf('=');
		if (eq < 0)
			continue; // skip lines without '='

		if (currentSection.isEmpty())
			continue; // skip lines before first section

		QString key = normalize(s.left(eq).trimmed());
		QString val = s.mid(eq + 1).trimmed();

		auto &section = m_data[currentSection];

		if (val.isEmpty()) {
			// Empty value signals key deletion
			if (section.contains(key)) {
				section.remove(key);
				qDebug().noquote()
					<< "[INI] removed"
					<< "[" << currentSection << "]"
					<< key;
			}
			continue;
		}

		// Remove surrounding quotes if present
		if (val.size() >= 2 && val.startsWith('"') && val.endsWith('"'))
			val = val.mid(1, val.size() - 2);

		auto it = section.find(key);
		if (it == section.end()) {
			section.insert(key, val);
			qDebug().noquote()
				<< "[INI] added"
				<< "[" << currentSection << "]"
				<< key << "=" << val;
		} else if (it.value() != val) {
			it.value() = val;
			qDebug().noquote()
				<< "[INI] updated"
				<< "[" << currentSection << "]"
				<< key << "=" << val;
		}
	}
}

QVariant IniStreamConfig::get(const QString &section, const QString &key, const QVariant &def) const
{
	auto sit = m_data.find(normalize(section));
	if (sit == m_data.end())
		return def;

	auto kit = sit->find(key);
	return (kit != sit->end()) ? *kit : def;
}

int IniStreamConfig::getInt(const QString &section, const QString &key, int def) const
{
	bool ok = false;
	int val = get(section, key, QVariant(def)).toString().toInt(&ok);
	return ok ? val : def;
}

double IniStreamConfig::getDouble(const QString &section, const QString &key, double def) const
{
	bool ok = false;
	double val = get(section, key, QVariant(def)).toString().toDouble(&ok);
	return ok ? val : def;
}

bool IniStreamConfig::getBool(const QString &section, const QString &key, bool def) const
{
	QString s = get(section, key, QVariant()).toString().trimmed().toLower();
	if (s == "1" || s == "true" || s == "yes" || s == "on")
		return true;
	if (s == "0" || s == "false" || s == "no" || s == "off")
		return false;
	return def;
}

bool IniStreamConfig::hasKey(const QString &section, const QString &key) const
{
	auto sit = m_data.find(normalize(section));
	if (sit == m_data.end())
		return false;
	return sit->contains(key);
}

bool IniStreamConfig::hasSection(const QString &section) const
{
	return m_data.contains(normalize(section));
}

QStringList IniStreamConfig::keys(const QString &section) const
{
	auto sit = m_data.find(normalize(section));
	if (sit == m_data.end())
		return {};
	return sit->keys();
}

QStringList IniStreamConfig::sections(const QString &prefix) const
{
	QStringList result;
	for (auto it = m_data.constBegin(); it != m_data.constEnd(); ++it) {
		if (prefix.isEmpty() || it.key().startsWith(prefix))
			result.append(it.key());
	}
	return result;
}
