#ifndef PROTOCOMPILER_HPP
#define PROTOCOMPILER_HPP

#include <yogi_cpp.hpp>

#include <QMap>
#include <QByteArray>
#include <QString>
#include <QTemporaryDir>


class ProtoCompiler
{
public:
    enum Language {
        LNG_PYTHON,
        LNG_CPP,
        LNG_CSHARP
    };

private:
    static ProtoCompiler* ms_instance;

    yogi::Logger m_logger;
    QString          m_executable;

private:
    void log_and_throw(const std::string& msg);
	void check_protoc_exists();
	void check_temp_dir_valid(const QTemporaryDir& dir);
	QString extract_proto_package(const QByteArray& protoFileContent);
	QString write_proto_file(const QTemporaryDir& dir, const QByteArray& protoFileContent);
	void run_protoc(const QTemporaryDir& dir, const QString& protoFilename, Language targetLanguage);
	QMap<QString, QByteArray> read_generated_files(const QTemporaryDir& dir, const QString& protoFilename);
	void post_process_generated_files(const QByteArray& protoFileContent, QMap<QString, QByteArray>* files);
	QString extract_signature_from_generated_file(const QByteArray& content);
	void escape_file_contents(QMap<QString, QByteArray>* files);
	void insert_before(QByteArray* content, const QString& str, const QString& where);

public:
    static ProtoCompiler& instance();

    ProtoCompiler();

    QMap<QString, QByteArray> compile(const QByteArray& protoFileContent, Language targetLanguage);
};

#endif // PROTOCOMPILER_HPP
