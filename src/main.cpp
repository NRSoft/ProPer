#include <iostream>
#include <QApplication>

#include "spdlog/spdlog.h"
#include "version.h"
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    int r = 0;

    try
    {
        auto fileSink = std::make_shared<spdlog::sinks::simple_file_sink_mt>("ProPer.log");
        auto mainLogger = std::make_shared<spdlog::logger>("main", fileSink);
        auto userLogger = std::make_shared<spdlog::logger>("user", fileSink); // view
        auto taskLogger = std::make_shared<spdlog::logger>("task", fileSink); // model
//        auto plotLogger = std::make_shared<spdlog::logger>("plot", fileSink);

        spdlog::register_logger(mainLogger);
        spdlog::register_logger(userLogger);
        spdlog::register_logger(taskLogger);
//        spdlog::register_logger(plotLogger);

        if(argc > 1){ // logging enabled
            mainLogger->flush_on(spdlog::level::err);
            userLogger->flush_on(spdlog::level::err);
            taskLogger->flush_on(spdlog::level::err);
    //        plotLogger->flush_on(spdlog::level::err);

            spdlog::set_pattern("[%H:%M:%S.%e][%n]%L: %v");

            QString timestamp = QDate::currentDate().toString(QLatin1String("ddd dd MMM yyyy"));
            mainLogger->info("---- [{}] starting ProPer ver {}.{} built on {} ----",
                             timestamp.toUtf8().data(), VERSION_MAJOR, VERSION_MINOR, VERSION_DATE);

            // set logging level from command line parameter (number from 0 to 6,
            //  defined in spdlog/common.h: trace=0, debug=1, info=2, warning=3, error=4, critical=5, off=6)
            // if no parameter present, default is "info"
            spdlog::set_level(static_cast<spdlog::level::level_enum>(atoi(argv[1]))); // globally
            //spdlog::set_level(spdlog::level::debug);

            userLogger->info("logger initialised");
            taskLogger->info("logger initialised");
    //        plotLogger->info("logger initialised");
        }
        else
            spdlog::set_level(spdlog::level::off);

        QApplication a(argc, argv);
        MainWindow w;
        w.show();

        r = a.exec();

        if(argc > 1){
            mainLogger->set_level(spdlog::level::info); // it may've been changed
            mainLogger->info("exiting ProPer with exit code {}", r);
        }
        spdlog::drop_all();
    }
    catch (const spdlog::spdlog_ex& ex)
    {
        std::cout << "SpdLog exception: " << ex.what() << std::endl;
    }

    return r;
}
