#pragma once

#include <QString>
#include <QMap>
#include <QVariant>
#include <QStringList>

class IniStreamConfig
{
public:
	IniStreamConfig() = default;

	// Parse a chunk of INI text; stateless
	void parse(const QByteArray &chunk);

	// Basic get
	QVariant get(const QString &section, const QString &key, const QVariant &def = QVariant()) const;

	// Typed getters
	int getInt(const QString &section, const QString &key, int def = 0) const;
	double getDouble(const QString &section, const QString &key, double def = 0.0) const;
	bool getBool(const QString &section, const QString &key, bool def = false) const;

	bool hasKey(const QString &section, const QString &key) const;
	bool hasSection(const QString &section) const;

	QStringList keys(const QString &section) const;
	QStringList sections(const QString &prefix = QString()) const;

private:
	static inline QString normalize(const QString &s) {
		return s.toLower();
	}
	QMap<QString, QMap<QString, QVariant>> m_data;
};
