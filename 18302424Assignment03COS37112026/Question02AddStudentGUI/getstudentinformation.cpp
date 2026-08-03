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

    // Initialize the external program by locating and copying it,
    // which also dynamically sets m_filePathExternalProgram.
    copyExternalProgram();
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
    // Read the standard output from the QProcess.
    QByteArray data = m_process.readAllStandardOutput();
    // Emit the signal to send the string to the GUI.
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

// Custom function to find the specific folder path.
QString GetStudentInformation::findCustomFilePath()
{
    // Start at the current QApplication build directory.
    QDir currentDir(QApplication::applicationDirPath());

    // Navigate upwards until we find the common main folder.
    while(currentDir.dirName() != "18302424Assignment03COS37112026" && !currentDir.isRoot())
    {
        currentDir.cdUp();
    }

    // Return the custom file path to Question 1's directory.
    return (currentDir.absolutePath() + "/Question01GetStudentInfo");
}

// Custom logic to copy the .exe and save its new location inside the QApplication build directory.
void GetStudentInformation::copyExternalProgram()
{
    question1FolderPath = findCustomFilePath();

    // Navigate downwards the build folder until we find the file called Question01GetStudentInfo.exe.
    // Define the source path of Question01GetStudentInfo.exe.[cite: 14]
    sourcePath = "";

    // Search recursively through all subdirectories for the specific executable file
    QDirIterator it(question1FolderPath, QStringList() << "Question01GetStudentInfo.exe", QDir::Files, QDirIterator::Subdirectories);

    while (it.hasNext())
    {
        sourcePath = it.next(); // Sets the absolute file path to the executable.
        break; // Exit the loop once the first matching file is found.
    }

    if (sourcePath.isEmpty())
    {
        qDebug() << "Error: Question01GetStudentInfo.exe could not be found in subdirectories.";
    }
    else
    {
        qDebug() << "Executable found at:" << sourcePath;

        // Define the destination path (Question 2's build folder)
        destinationPath = QApplication::applicationDirPath();

        // Construct the full destination path including the file name
        QString fullDestinationFile = destinationPath + "/Question01GetStudentInfo.exe";

        // Remove the existing file if it is already there to allow overwriting
        if (QFile::exists(fullDestinationFile))
        {
            QFile::remove(fullDestinationFile);
        }

        // Copy the executable to the new destination
        if (QFile::copy(sourcePath, fullDestinationFile))
        {
            qDebug() << "Successfully copied Question01GetStudentInfo.exe to:" << fullDestinationFile;

            // Update the internal path variable to point to the newly copied local executable
            m_filePathExternalProgram = fullDestinationFile;
        }
        else
        {
            qDebug() << "Error: Failed to copy the executable to the destination path.";
        }
    }
}