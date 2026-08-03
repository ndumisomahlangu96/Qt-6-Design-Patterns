#ifndef GETSTUDENTINFORMATION_H
#define GETSTUDENTINFORMATION_H

#include <QObject>
#include <QDebug>
#include <QProcess>
#include <QDir>
#include <QFileDialog>

class GetStudentInformation : public QObject
{
    Q_OBJECT
public:
    explicit GetStudentInformation(QObject *parent = nullptr);

signals:
    // Update the signal to pass the captured output
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
    QString m_filePathExternalProgram;

};

#endif // GETSTUDENTINFORMATION_H
