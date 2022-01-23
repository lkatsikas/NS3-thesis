/* Create net.xml from osm */
netconvert --osm-files Manhattan.osm -o Manhattan.net.xml

/* Polyconvert */
polyconvert --net-file Manhattan.net.xml --osm-files Manhattan.osm --type-file typemap.xml -o Manhattan.poly.xml

/* Generate rou.xml */
python /home/katsikas/wns3-2015/ns-allinone-3.21/sumo-0.23.0/NS3-Example/sumo-0.23.0/tools/trip/randomTrips.py -n Manhattan.net.xml -r Manhattan.rou.xml -e 55.0 -p 0.15 --trip-attributes="departLane=\"best\" departSpeed=\"max\" departPos=\"free\"" --intermediate 4 -l

/* See in GUI */
sumo-gui -c Manhattan.sumo.cfg



/* Produce fcd */
sumo -n Manhattan.net.xml -r Manhattan.rou.xml --fcd-output Manhattan.xml

/* Produce mobility.tcl ns-3 file */
../../tools/traceExporter.py --fcd-input=Manhattan.xml --ns2mobility-output=mobility.tcl

/* Produce activity.tcl ns-3 file */
../../tools/traceExporter.py --fcd-input=Manhattan.xml --ns2mobility-output=activity.tcl

/* Produce config.tcl ns-3 file */
../../tools/traceExporter.py --fcd-input=Manhattan.xml --ns2mobility-output=config.tcl
