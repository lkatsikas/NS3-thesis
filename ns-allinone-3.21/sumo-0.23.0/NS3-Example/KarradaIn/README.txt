/* Create net.xml from osm */
netconvert --osm-files KarradaIn.osm -o KarradaIn.net.xml

/* Polyconvert */
polyconvert --net-file KarradaIn.net.xml --osm-files KarradaIn.osm --type-file typemap.xml -o KarradaIn.poly.xml

/* Generate rou.xml */
python /home/katsikas/wns3-2015/ns-allinone-3.21/sumo-0.23.0/NS3-Example/sumo-0.23.0/tools/trip/randomTrips.py -n KarradaIn.net.xml -r KarradaIn.rou.xml -e 45.0 -p 0.5 --intermediate 3 -l

/* See in GUI */
sumo-gui -c KarradaIn.sumo.cfg



/* Produce fcd */
sumo -n KarradaIn.net.xml -r KarradaIn.rou.xml --fcd-output KarradaIn.xml

/* Produce mobility.tcl ns-3 file */
../../tools/traceExporter.py --fcd-input=KarradaIn.xml --ns2mobility-output=mobility.tcl

/* Produce activity.tcl ns-3 file */
../../tools/traceExporter.py --fcd-input=KarradaIn.xml --ns2mobility-output=activity.tcl

/* Produce config.tcl ns-3 file */
../../tools/traceExporter.py --fcd-input=KarradaIn.xml --ns2mobility-output=config.tcl



/* Run */
./waf --run "src/v2v/examples/v2v-novel-algorithm-example --traceFile=/home/katsikas/wns3-2015/ns-allinone-3.21/sumo-0.23.0/NS3-Example/KarradaIn/mobility.tcl --ueNumber=82 --simTime=90 --range=high --logFile=ns3-mob.log"
