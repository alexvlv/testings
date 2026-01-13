                                                                                                                                   #include "IniStreamConfig.h"

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
			currentSection = s.mid(1, s.size() - 2).trimmed();
			continue;
		}

		int eq = s.indexOf('=');
		if (eq < 0) continue; // skip lines without '='

		QString key = s.left(eq).trimmed();
		QString val = s.mid(eq + 1).trimmed();

		if (currentSection.isEmpty())
			continue; // skip lines before first section

		if (val.isEmpty()) {
			// Empty value signals key deletion
			m_data[currentSection].remove(key);
		} else {
			// Remove surrounding quotes if present
			if (val.size() >= 2 && val.startsWith('"') && val.endsWith('"'))
				val = val.mid(1, val.size() - 2);

			m_data[currentSection][key] = val;
		}
	}
}

QVariant IniStreamConfig::get(const QString &section, const QString &key, const QVariant &def) const
{
	auto sit = m_data.find(section);
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
	auto sit = m_data.find(section);
	if (sit == m_data.end()) return false;
	return sit->contains(key);
}

// Return all keys of a section, empty list if section not present
QStringList IniStreamConfig::keys(const QString &section) const
{
	auto sit = m_data.find(section);
	if (sit == m_data.end()) return {};
	return sit->keys();
}
