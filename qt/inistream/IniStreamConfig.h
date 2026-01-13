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

	// Check if key exists
	bool hasKey(const QString &section, const QString &key) const;

	// Get all keys of a section
	QStringList keys(const QString &section) const;

private:
	QMap<QString, QMap<QString, QVariant>> m_data;
};
