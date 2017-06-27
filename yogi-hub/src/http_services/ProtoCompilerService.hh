#ifndef HTTP_SERVICES_PROTOCOMPILERSERVICE_HPP
#define HTTP_SERVICES_PROTOCOMPILERSERVICE_HPP

#include "Service.hh"

#include <yogi.hpp>

#include <QTemporaryDir>


namespace http_services {

class ProtoCompilerService : public Service
{
public:
    ProtoCompilerService();

    virtual void async_handle_request(request_type, const QString& path,
        const QMap<QString, QString>& header, const QByteArray& content, completion_handler) override;

private:
    enum Language {
        LNG_NONE,
        LNG_PYTHON,
        LNG_CPP,
        LNG_CSHARP
    };

    yogi::Logger m_logger;
    QString      m_executable;

    QMap<QString, QByteArray> compile(const QByteArray& protoFileContent, Language targetLanguage);
    void log_and_throw(const std::string& msg);
	void check_protoc_exists();
	void check_temp_dir_valid(const QTemporaryDir& dir);
	QString extract_proto_package(const QByteArray& protoFileContent);
    QString extract_innermost_proto_package(const QByteArray& protoFileContent);
	QString write_proto_file(const QTemporaryDir& dir, const QByteArray& protoFileContent);
	void run_protoc(const QTemporaryDir& dir, const QString& protoFilename, Language targetLanguage);
	QMap<QString, QByteArray> read_generated_files(const QTemporaryDir& dir, const QString& protoFilename);
	void post_process_generated_files(const QByteArray& protoFileContent, QMap<QString, QByteArray>* files);
	QString extract_signature_from_generated_file(const QByteArray& content);
	void escape_file_contents(QMap<QString, QByteArray>* files);
	void insert_before(QByteArray* content, const QString& str, const QString& where);
    void insert_after(QByteArray* content, const QString& str, const QString& where);
};

} // namespace http_services

#endif // HTTP_SERVICES_PROTOCOMPILERSERVICE_HPP
