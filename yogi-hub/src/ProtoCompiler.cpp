#include "ProtoCompiler.hpp"
#include "helpers.hpp"

#include <QDebug>
#include <QDir>
#include <QTemporaryDir>
#include <QFileInfo>
#include <QProcess>
#include <QRegExp>

#include <exception>
#include <cassert>
#include <string>
using namespace std::string_literals;


ProtoCompiler* ProtoCompiler::ms_instance = nullptr;

void ProtoCompiler::log_and_throw(const std::string& msg)
{
    YOGI_LOG_ERROR(m_logger, msg);
    throw std::runtime_error(msg);
}

void ProtoCompiler::check_protoc_exists()
{
	if (!QFile::exists(m_executable)) {
        log_and_throw("Cannot find compiler executable '"s + m_executable.toStdString() + "'");
	}
}

void ProtoCompiler::check_temp_dir_valid(const QTemporaryDir& dir)
{
	if (!dir.isValid()) {
        log_and_throw("Cannot create temporary directory '"s + dir.path().toStdString() + "'");
	}
}

QString ProtoCompiler::extract_proto_package(const QByteArray& protoFileContent)
{
	QRegExp rx("package ([\\w+\\.]*yogi_([0-9a-f]{8}));", Qt::CaseInsensitive);
	rx.setMinimal(true);
	if (rx.indexIn(protoFileContent) == -1) {
		log_and_throw("Could not extract proto package from generated file");
	}

	return rx.cap(1);
}

QString ProtoCompiler::extract_innermost_proto_package(const QByteArray& protoFileContent)
{
	auto package = extract_proto_package(protoFileContent);
	return package.mid(package.lastIndexOf("yogi_"));
}

QString ProtoCompiler::write_proto_file(const QTemporaryDir& dir, const QByteArray& protoFileContent)
{
	auto filename = extract_proto_package(protoFileContent).replace('.', "__") + ".proto";

	QFile protoFile(QDir(dir.path()).filePath(filename));
	protoFile.open(QIODevice::WriteOnly | QIODevice::Text);
	if (!protoFile.isOpen()) {
        auto msg = "Cannot create temporary file '"s + filename.toStdString() + "' in '"
            + dir.path().toStdString() + "': " + protoFile.errorString().toStdString();
        log_and_throw(msg);
	}

	protoFile.write(protoFileContent);
	protoFile.close();

	return filename;
}

void ProtoCompiler::run_protoc(const QTemporaryDir& dir, const QString& protoFilename, Language targetLanguage)
{
	QStringList args;
	args << protoFilename;
	switch (targetLanguage) {
	case LNG_PYTHON:
		args << "--python_out=.";
        YOGI_LOG_INFO(m_logger, "Generating Python files...");
		break;

	case LNG_CPP:
		args << "--cpp_out=.";
        YOGI_LOG_INFO(m_logger, "Generating C++ files...");
		break;

	case LNG_CSHARP:
		args << "--csharp_out=.";
        YOGI_LOG_INFO(m_logger, "Generating C# files...");
		break;
	}

	QProcess protoc;
	protoc.setWorkingDirectory(dir.path());
	protoc.start(m_executable, args);

	if (!protoc.waitForStarted(1000)) {
		log_and_throw("Could not start protoc: "s + protoc.errorString().toStdString());
	}

	if (!protoc.waitForFinished(3000)) {
		protoc.kill();
		log_and_throw("Waiting for protoc to finish failed: "s + protoc.errorString().toStdString());
	}

	if (protoc.exitCode() != 0) {
		auto output = protoc.readAllStandardError();
		log_and_throw("Compilation failed:\n"s + output.data());
	}
}

QMap<QString, QByteArray> ProtoCompiler::read_generated_files(const QTemporaryDir& dir, const QString& protoFilename)
{
    YOGI_LOG_TRACE(m_logger, "Reading generated files...");
	auto genFiles = QDir(dir.path()).entryList();
	genFiles.removeOne(".");
	genFiles.removeOne("..");
	genFiles.removeOne(protoFilename);

    auto filenameBase = protoFilename.left(protoFilename.lastIndexOf('.'));

	QMap<QString, QByteArray> files;
	for (auto& filename : genFiles) {
		QFile file(QDir(dir.path()).filePath(filename));
		file.open(QIODevice::ReadOnly | QIODevice::Text);

		auto extension = QFileInfo(file).suffix().prepend('.');
        if (extension == ".py") {
            extension.prepend("_pb2");
        }

		files[filenameBase + extension] = file.readAll();
		file.close();
	}

	return files;
}

void ProtoCompiler::post_process_generated_files(const QByteArray& protoFileContent, QMap<QString, QByteArray>* files)
{
    YOGI_LOG_TRACE(m_logger, "Post-processing generated files...");

	auto package = extract_innermost_proto_package(protoFileContent);

	for (auto& filename : files->keys()) {
		auto filenameBase = filename.left(filename.indexOf("."));
		auto shortFilenameBase = filenameBase.mid(filenameBase.lastIndexOf("yogi_"));

		auto& content = (*files)[filename];
		auto signature = extract_signature_from_generated_file(content);

		if (filename.endsWith(".py")) {
			content += "ScatterMessage.SIGNATURE = 0x" + signature + "\r\n";
			content += "GatherMessage.SIGNATURE = 0x" + signature + "\r\n";
			content += "PublishMessage.SIGNATURE = 0x" + signature + "\r\n";
			content += "MasterMessage.SIGNATURE = 0x" + signature + "\r\n";
			content += "SlaveMessage.SIGNATURE = 0x" + signature + "\r\n";
		}
		else if (filename.endsWith(".h")) {
			content.replace(QByteArray(":") + package, QByteArray(":") + package.toUtf8() + "_ns");
			content.replace(QByteArray("namespace ") + package, QByteArray("namespace ") + package.toUtf8() + "_ns");
			QString structStr;
			structStr += "\n\nstruct " + package + "\n";
			structStr += "{\n";
			structStr += "    typedef " + package + "_ns::PublishMessage PublishMessage;\n";
			structStr += "    typedef " + package + "_ns::ScatterMessage ScatterMessage;\n";
			structStr += "    typedef " + package + "_ns::GatherMessage  GatherMessage;\n";
			structStr += "    typedef " + package + "_ns::MasterMessage  MasterMessage;\n";
			structStr += "    typedef " + package + "_ns::SlaveMessage   SlaveMessage;\n";
			structStr += "\n";
			structStr += "    enum {\n";
			structStr += "        SIGNATURE = 0x" + signature + "\n";
			structStr += "    };\n";
			structStr += "};";
			insert_after(&content, structStr, QString("}  // namespace ") + package + "_ns");
		}
		else if (filename.endsWith(".cc")) {
			content.replace(QByteArray(":") + package, QByteArray(":") + package.toUtf8() + "_ns");
			content.replace(QByteArray("namespace ") + package, QByteArray("namespace ") + package.toUtf8() + "_ns");
			content.replace(package + ".pb.", package.toUtf8() + ".");
			content.replace(QByteArray("#include \"") + filenameBase.toUtf8() + ".h\"", QByteArray("#include \"") + shortFilenameBase.toUtf8() + ".h\"");
		}
		else if (filename.endsWith(".cs")) {
			auto sigStr = QString("public const int SIGNATURE = 0x") + signature + ";\r\n    ";
			// TODO: This does not look like it would work
			insert_before(&content, sigStr, "public ScatterMessage()");
			insert_before(&content, sigStr, "public GatherMessage()");
			insert_before(&content, sigStr, "public PublishMessage()");
			insert_before(&content, sigStr, "public MasterMessage()");
			insert_before(&content, sigStr, "public SlaveMessage()");
		}

		if (shortFilenameBase != filenameBase) {
			(*files)[filename.mid(filename.lastIndexOf("yogi_"))] = content;
			files->remove(filename);
		}
	}
}

QString ProtoCompiler::extract_signature_from_generated_file(const QByteArray& content)
{
    YOGI_LOG_TRACE(m_logger, "Extracting signature from generated files...");

	QRegExp rx("yogi_([0-9a-f]{8})", Qt::CaseInsensitive);
	if (rx.indexIn(content) == -1) {
		log_and_throw("Could not extract signature from generated files");
	}

	return rx.cap(1);
}

void ProtoCompiler::escape_file_contents(QMap<QString, QByteArray>* files)
{
	for (auto& filename : files->keys()) {
		(*files)[filename].replace('\\', "\\\\").replace('\r', "\\r").replace('\n', "\\n");
	}
}

void ProtoCompiler::insert_before(QByteArray* content, const QString& str, const QString& where)
{
	auto pos = content->indexOf(where);
	if (pos == -1) {
		log_and_throw("Could not find '"s + where.toStdString() + "' in generated file");
	}

	content->insert(pos, str);
}

void ProtoCompiler::insert_after(QByteArray* content, const QString& str, const QString& where)
{
	auto pos = content->indexOf(where);
	if (pos == -1) {
		log_and_throw("Could not find '"s + where.toStdString() + "' in generated file");
	}

	content->insert(pos + where.size(), str);
}

ProtoCompiler& ProtoCompiler::instance()
{
    assert (ms_instance != nullptr);
    return *ms_instance;
}

ProtoCompiler::ProtoCompiler()
: m_logger("Proto Compiler")
{
    if (yogi::ProcessInterface::config().get<bool>("proto-compiler.enabled")) {
        m_executable = QString::fromStdString(yogi::ProcessInterface::config().get<std::string>("proto-compiler.executable"));
        check_protoc_exists();
        YOGI_LOG_INFO(m_logger, "Proto compiler enabled");
    } else {
        YOGI_LOG_DEBUG(m_logger, "Disabled Proto compiler");
    }

    assert (ms_instance == nullptr);
    ms_instance = this;
}

QMap<QString, QByteArray> ProtoCompiler::compile(const QByteArray& protoFileContent, Language targetLanguage)
{
    if (!yogi::ProcessInterface::config().get<bool>("proto-compiler.enabled")) {
        YOGI_LOG_INFO(m_logger, "Denied request since compiler is disabled");
        throw std::runtime_error("Proto compiler is disabled");
    }

	check_protoc_exists();

    QTemporaryDir dir;
	check_temp_dir_valid(dir);

	auto protoFilename = write_proto_file(dir, protoFileContent);
	run_protoc(dir, protoFilename, targetLanguage);
	auto files = read_generated_files(dir, protoFilename);
	post_process_generated_files(protoFileContent, &files);
	escape_file_contents(&files);

    return files;
}
