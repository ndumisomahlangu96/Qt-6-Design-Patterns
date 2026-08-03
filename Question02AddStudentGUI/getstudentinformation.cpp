#include "getstudentinformation.h"

GetStudentInformation::GetStudentInformation(QObject *parent)
    : QObject{parent}
{
    connect(&m_process,&QProcess::errorOccurred,this,&GetStudentInformation::errorOccurred);
    connect(&m_process,&QProcess::readyReadStandardError,this,&GetStudentInformation::readyReadStandardError);
    connect(&m_process,&QProcess::readyReadStandardOutput,this,&GetStudentInformation::readyReadStandardOutput);
    connect(&m_process,&QProcess::started,this,&GetStudentInformation::started);
    connect(&m_process,&QProcess::stateChanged,this,&GetStudentInformation::stateChanged);
    connect(&m_process,QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),this,&GetStudentInformation::finished);

    // Assumes Question01GetStudentInfo.exe is placed in the Question02AddStudentGUI build folder
    m_filePathExternalProgram = "./Question01GetStudentInfo.exe";
}

void GetStudentInformation::startSeparateProgram()
{
    m_process.start(m_filePathExternalProgram);
}

void GetStudentInformation::stopSeparateProgram()
{
    if(m_process.state() == QProcess::Running) {
        m_process.terminate();
    }
}

void GetStudentInformation::errorOccurred(QProcess::ProcessError error)
{
    qDebug() << "Process Error:" << error;
}

void GetStudentInformation::finished(int exitCode, QProcess::ExitStatus exitStatus)
{
    qDebug() << "Process Finished. Exit Code:" << exitCode;
}

void GetStudentInformation::readyReadStandardError()
{
    QByteArray data = m_process.readAllStandardError();
    emit outputGUI(QString("Error: ") + QString::fromLocal8Bit(data));
}

void GetStudentInformation::readyReadStandardOutput()
{
    // Read the standard output from the QProcess
    QByteArray data = m_process.readAllStandardOutput();
    // Emit the signal to send the string to the GUI
    emit outputGUI(QString::fromLocal8Bit(data));
}

void GetStudentInformation::started()
{
    qDebug() << "Process successfully started.";
}

void GetStudentInformation::stateChanged(QProcess::ProcessState newState)
{
    qDebug() << "Process State:" << newState;
}
