#ifndef GETSTUDENTINFORMATION_H
#define GETSTUDENTINFORMATION_H

#include <QObject>
#include <QDebug>
#include <QProcess>
#include <QDir>
#include <QApplication>
#include <QFile>
#include <QDirIterator>

class GetStudentInformation : public QObject
{
    Q_OBJECT
public:
    explicit GetStudentInformation(QObject *parent = nullptr);

signals:
    // Update the signal to pass the captured output.
    void outputGUI(const QString &output);

public slots:
    void startSeparateProgram();
    void stopSeparateProgram();

private slots:
    void errorOccurred(QProcess::ProcessError error);
    void finished(int exitCode, QProcess::ExitStatus exitStatus);
    void readyReadStandardError();
    void readyReadStandardOutput();
    void started();
    void stateChanged(QProcess::ProcessState newState);

private:
    QProcess m_process;

    // Custom string variable to hold file path of external QApplication program.
    QString m_filePathExternalProgram;
    QString question1FolderPath;
    QString sourcePath;
    QString destinationPath;
    QString findCustomFilePath();
    void copyExternalProgram();

};

#endif // GETSTUDENTINFORMATION_H