/* Create net.xml from osm */
netconvert --osm-files Batavia.osm -o Batavia.net.xml

/* Polyconvert */
polyconvert --net-file Batavia.net.xml --osm-files Batavia.osm --type-file typemap.xml -o Batavia.poly.xml

/* Generate rou.xml */
python /home/katsikas/wns3-2015/ns-allinone-3.21/sumo-0.23.0/NS3-Example/sumo-0.23.0/tools/trip/randomTrips.py -n Batavia.net.xml -r Batavia.rou.xml -e 50.0 -p 0.5 --trip-attributes="departLane=\"best\" departSpeed=\"max\" departPos=\"free\"" -l

/* See in GUI */
sumo-gui -c Batavia.sumo.cfg



/* Produce fcd */
sumo -n Batavia.net.xml -r Batavia.rou.xml --fcd-output Batavia.xml

/* Produce mobility.tcl ns-3 file */
../../tools/traceExporter.py --fcd-input=Batavia.xml --ns2mobility-output=mobility.tcl

/* Produce activity.tcl ns-3 file */
../../tools/traceExporter.py --fcd-input=Batavia.xml --ns2mobility-output=activity.tcl

/* Produce config.tcl ns-3 file */
../../tools/traceExporter.py --fcd-input=Batavia.xml --ns2mobility-output=config.tcl
