# -------------------------------------------------
# Project created by QtCreator 2009-04-26T13:09:27
# -------------------------------------------------
# include(../../mani-thesis/mlearner/mlearner.pro)
QT += network \
    opengl \
    xml \
    script \
    scripttools \
    sql
TARGET = ai
TEMPLATE = app
SOURCES += main.cpp \
    util/Object.cpp \
    util/WMatching.cpp \
    worldmodel.cpp \
    mainapplication.cpp \
    agent.cpp \
    Field.cpp \
    coach.cpp \
    visionclient.cpp \
    gamestate.cpp \
    communicator.cpp \
    drawer.cpp \
    planner.cpp \
    obstacle.cpp \
    curve.cpp \
    motiontrainer.cpp \
    statemachine.cpp \
    mathtools.cpp \
    motionestimator.cpp \
    logger.cpp \
    profiler.cpp \
    net/netraw.cpp \
    proto/messages_robocup_ssl_wrapper.pb.cc \
    proto/messages_robocup_ssl_refbox_log.pb.cc \
    proto/messages_robocup_ssl_geometry.pb.cc \
    proto/messages_robocup_ssl_detection.pb.cc \
    geom/geom.cpp \
    ui/widgets.cpp \
    ui/varswidget.cpp \
    skills/skill.cpp \
    skills/gotopoint.cpp \
    skills/trackcurve.cpp \
    skills/kick.cpp \
    skills/gotoball.cpp \
    robot.cpp \
    ball.cpp \
    movingobject.cpp \
    team.cpp \
    trajectoryplanner.cpp \
    kdtree.cpp \
    roles/role.cpp \
    knowledge.cpp \
    skills/spin.cpp \
    roles/stop.cpp \
    conditions.cpp \
    strategy.cpp \
    roles/mark.cpp \
    roles/playmake.cpp \
    roles/position.cpp \
    roles/block.cpp \
    skills/hittheball.cpp \
    ui/skillwidgets.cpp \
    ui/rolewidgets.cpp \
    util/pid.cpp \
    statisticalanalysis.cpp \
    learning/policylearner.cpp \
    passevaluation.cpp \
    util/NNFPP.cpp \
    util/MLP.cpp \
    soccer.cpp \
    net/robocup_ssl_server.cpp \
    net/robocup_ssl_client.cpp \
    behaviours/behaviour.cpp \
    util/robottracker.cpp \
    util/tracker.cpp \
    util/matrix.cpp \
    util/kalman.cpp \
    util/balltracker.cpp \
    opponent.cpp \
    graphWidget.cpp \
    roles/warmup.cpp \
    proto/grSim_Replacement.pb.cc \
    proto/grSim_Packet.pb.cc \
    proto/grSim_Commands.pb.cc \
    automatedreferee.cpp \
    joystick.cpp \
    util/gamelogger.cpp \
    behaviours/offensive.cpp \
    callibration.cpp \
    defensepositioning.cpp \
    roles/support.cpp \
    simulation/simulator.cpp \
    plans/plan.cpp \
    plans/markplan.cpp \
    plans/defenseplan.cpp \
    exceptions.cpp \
    navigation.cpp \
    roles/gaurd.cpp \
    util/hysteresisedcheck.cpp \
    plans/tikitakaplan.cpp \
    plans/khersdefense.cpp \
    newbangbang.cpp \
    technicalchallenge.cpp \
    geom/voronoi_diagram_triangle.cpp \
    geom/voronoi_diagram_original.cpp \
    geom/triangulation.cpp \
    geom/delaunay_triangulation.cpp \
    geom/triangle/triangle.c \
    formation/sample_data.cpp \
    formation/formation_uva.cpp \
    formation/formation_static.cpp \
    formation/formation_dt.cpp \
    formation/formation_cdt.cpp \
    formation/formation.cpp \
    formation/edit_data.cpp \
    formation/options.cpp \
    formation/rcss_param_parser.cpp \
    formation/param_map.cpp \
    formation/conf_file_parser.cpp \
    formation/cmd_line_parser.cpp \
    formation/formation_ssl.cpp \
    intentions/intention_defense.cpp \
    intentions/intention_mark.cpp \
    intentions/intention_playmake.cpp \
    intentions/intention_block.cpp \
    intentions/intention_position.cpp \
    plays/masterplay.cpp \
    plays/ourkickoff.cpp \
    plays/ourindirect.cpp \
    plays/ourdirect.cpp \
    plays/ourpenalty.cpp \
    plays/theirkickoff.cpp \
    plays/theirdirect.cpp \
    plays/theirindirect.cpp \
    plays/theirpenalty.cpp \
    plays/forcestart.cpp \
    plans/positioningplan.cpp \
    plays/doublesizeourdirect.cpp \
    roles/playonrole.cpp \
    plays/playoff.cpp \
    proto/radio_protocol_command.pb.cc \
    proto/radio_protocol_wrapper.pb.cc \
    skills/srskills.cpp \
    recorder.cpp \
    plays/dynamicattack.cpp \
    roles/playoffrole.cpp \
    roles/dynamicrole.cpp \
    skillsthread.cpp \
    tools/planloader.cpp \
    tools/loadplayoffjson.cpp \
    util/filewatcher.cpp \
    proto/referee.pb.cc \
    plays/ourballplacement.cpp \
    plays/theirballplacement.cpp\
    skills/autoballplacement.cpp \ 
    util/timer.cpp \
    util/rng.cpp \
    collectprofiledata.cpp \
    plays/stopplay.cpp \
    defpos.cpp \
    chipkick.cpp \
    mixteamthread.cpp \
    proto/multi_team_communication.pb.cc \
    mixteamsender.cpp \
    mixteamreader.cpp \
    mixteamhandler.cpp

HEADERS += base.h \
    worldmodel.h \
    mainapplication.h \
    drawer.h \
    agent.h \
    gamestate.h \
    commands.h \
    Field.h \
    coach.h \
    visionclient.h \
    planner.h \
    kdtree.h \
    obstacle.h \
    curve.h \
    communicator.h \
    geom.h \
    statemachine.h \
    mathtools.h \
    motionestimator.h \
    logger.h \
    profiler.h \
    net/netraw.h \
    proto/messages_robocup_ssl_wrapper.pb.h \
    proto/messages_robocup_ssl_refbox_log.pb.h \
    proto/messages_robocup_ssl_geometry.pb.h \
    proto/messages_robocup_ssl_detection.pb.h \
    motiontrainer.h \
    ui/varswidget.h \
    experimental1.h \
    experimental2.h \
    experimental3.h \
    experimental4.h \
    experimental5.h \
    experimental6.h \
    skills/skill.h \
    skills/gotopoint.h \
    skills/trackcurve.h \
    skills/kick.h \
    skills/gotoball.h \
    robot.h \
    ball.h \
    movingobject.h \
    team.h \
    skills/skills.h \
    trajectoryplanner.h \
    roles/role.h \
    knowledge.h \
    roles/roles.h \
    skills/spin.h \
    roles/stop.h \
    conditions.h \
    roles/mark.h \
    roles/playmake.h \
    roles/position.h \
    util/SmartPointer.h \
    util/Object.h \
    roles/block.h \
    skills/hittheball.h \
    ui/widgets.h \
    util/pid.h \
    statisticalanalysis.h \
    learning/policylearner.h \
    passevaluation.h \
    soccer.h \
    net/robocup_ssl_server.h \
    net/robocup_ssl_client.h \
    behaviours/behaviour.h \
    util/robottracker.h \
    util/tracker.h \
    util/matrix.h \
    util/kalman.h \
    util/balltracker.h \
    util/WMatching.h \
    opponent.h \
    graphWidget.h \
    roles/warmup.h \
    proto/grSim_Replacement.pb.h \
    proto/grSim_Packet.pb.h \
    proto/grSim_Commands.pb.h \
    automatedreferee.h \
    include/ul.h \
    include/js.h \
    joystick.h \
    util/gamelogger.h \
    behaviours/offensive.h \
    callibration.h \
    defensepositioning.h \
    roles/support.h \
    simulation/simulator.h \
    plans/plan.h \
    plans/markplan.h \
    plans/defenseplan.h \
    exceptions.h \
    plans/plans.h \
    navigation.h \
    roles/gaurd.h \
    util/hysteresisedcheck.h \
    plans/tikitakaplan.h \
    plans/khersdefense.h \
    newbangbang.h \
    technicalchallenge.h \
    geom/voronoi_diagram_triangle.h \
    geom/voronoi_diagram_original.h \
    geom/voronoi_diagram.h \
    geom/triangulation.h \
    geom/delaunay_triangulation.h \
    geom/triangle/triangle.h \
    formation/sample_data.h \
    formation/formation_uva.h \
    formation/formation_static.h \
    formation/formation_sbsp.h \
    formation/formation_dt.h \
    formation/formation_cdt.h \
    formation/formation.h \
    formation/factory.h \
    formation/edit_data.h \
    formation/random.h \
    formation/math_util.h \
    formation/options.h \
    formation/rcss_param_parser.h \
    formation/param_parser.h \
    formation/param_map.h \
    formation/conf_file_parser.h \
    formation/cmd_line_parser.h \
    formation/formation_ssl.h \
    intentions/intentions.h \
    intentions/soccer_intention.h \
    intentions/intention_defense.h \
    intentions/intention_mark.h \
    intentions/intention_playmake.h \
    intentions/intention_block.h \
    intentions/intention_position.h \
    mergeCamerasExperiment.h \
    plays/masterplay.h \
    plays/ourkickoff.h \
    plays/ourindirect.h \
    plays/ourdirect.h \
    plays/ourpenalty.h \
    plays/theirkickoff.h \
    plays/theirdirect.h \
    plays/theirindirect.h \
    plays/theirpenalty.h \
    plays/forcestart.h \
    plans/positioningplan.h \
    plays/doublesizeourdirect.h \
    roles/playonrole.h \
    plays/playoff.h \
    proto/radio_protocol_command.pb.h \
    proto/radio_protocol_wrapper.pb.h \
    skills/srskills.h \
    recorder.h \
    joystickhandy.h \
    kickprofiler.h \
    plays/dynamicattack.h \
    roles/playoffrole.h \
    roles/dynamicrole.h \
    skillsthread.h \
    tools/planloader.h \
    tools/loadplayoffjson.h \
    proto/referee.pb.h \
    plays/plays.h \
    plays/theirballplacement.h \
    plays/ourballplacement.h \
    skills/autoballplacement.h \
    util/filewatcher.h \
    util/timer.h \
    util/rng.h \
    util/vector2.h \
    collectprofiledata.h \
    plays/stopplay.h \
    defpos.h \
    chipkick.h \
    mixteamthread.h \
    proto/multi_team_communication.pb.h \
    mixteamsender.h \
    mixteamreader.h \
    MixTeamChallenge.h \
    mixteamhandler.h

unix:INCLUDEPATH += "/usr/local/include"
unix:LIBS += -L/usr/local/include/qjson -lqjson

unix:!macx:!contains(QMAKE_HOST.arch, x86_64) {
message("Using libs")

LIBS += -L$$PWD/libs \
    -L/usr/lib/nvidia-current \
    -lPredictor \
    -lprotobuf \
    -lprotobuf-lite \
    -lqextserialport \
    -lVarTypes \
    -lsvm \
    -lplibjs \
    -lplibul
OBJECTS_DIR = $$PWD/objs
MOC_DIR = $$PWD/objs
INCLUDEPATH += $$PWD/
INCLUDEPATH += $$PWD/ui
INCLUDEPATH += $$PWD/plans/
INCLUDEPATH += $$PWD/skills/
INCLUDEPATH += $$PWD/roles/
INCLUDEPATH += $$PWD/include/
INCLUDEPATH += $$PWD/include/VarTypes
INCLUDEPATH += $$PWD/util
INCLUDEPATH += $$PWD/../../mani-thesis/mlearner/include
INCLUDEPATH += $$PWD/plays/
INCLUDEPATH += $$PWD/intentions/
}
macx {
LIBS += -L/usr/local/lib \
    -L$$PWD/macosx-libs \
    -lprotobuf \
    -lqextserialportd \
    -lVarTypes-0.6
OBJECTS_DIR = $$PWD/macosx-objs
MOC_DIR = $$PWD/macosx-objs
INCLUDEPATH += $$PWD/
INCLUDEPATH += $$PWD/ui
INCLUDEPATH += $$PWD/skills/
INCLUDEPATH += $$PWD/roles/
INCLUDEPATH += $$PWD/include/
INCLUDEPATH += $$PWD/
INCLUDEPATH += $$PWD/include/VarTypes
INCLUDEPATH += $$PWD/util
INCLUDEPATH += $$PWD/../../mani-thesis/mlearner/include
INCLUDEPATH += $$PWD/plays/
INCLUDEPATH += $$PWD/intentions/
}
contains(QMAKE_HOST.arch, x86_64):!macx {
message("Using lib64")
LIBS += -L$$PWD/lib64 \
    -L/usr/lib/nvidia-current \
    -lprotobuf \
    -lprotobuf-lite \
    -lprotoc \
    -lqextserialport \
    -lplibjs \
    -lplibul \
    -lVarTypes
OBJECTS_DIR = $$PWD/objs
MOC_DIR = $$PWD/objs
INCLUDEPATH += $$PWD/
INCLUDEPATH += $$PWD/ui
INCLUDEPATH += $$PWD/skills/
INCLUDEPATH += $$PWD/roles/
INCLUDEPATH += $$PWD/include/
INCLUDEPATH += $$PWD/
INCLUDEPATH += $$PWD/include/VarTypes
INCLUDEPATH += $$PWD/util
INCLUDEPATH += $$PWD/../../mani-thesis/mlearner/include
INCLUDEPATH += $$PWD/plays/
INCLUDEPATH += $$PWD/intentions/
}

RESOURCES +=
CONFIG += c++11

OTHER_FILES += \
    proto/pb/multi_team_communication.proto














