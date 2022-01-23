/* Create net.xml */
netconvert -n scenario2.nod.xml -e scenario2.edg.xml -t scenario2.typ.xml -o output/scenario2.net.xml

/* See net in GUI */
../../src/sumo-gui -n output/scenario2.net.xml

/* See net + rou in GUI */
../../src/sumo-gui -n output/scenario2.net.xml -r scenario2.rou.xml

/* Produce fcd */
sumo -n output/scenario2.net.xml -r scenario2.rou.xml --fcd-output output/fcd.xml

/* Produce mobility.tcl ns-3 file */
../../tools/traceExporter.py --fcd-input=output/fcd.xml --ns2mobility-output=output/mobility.tcl

/* Produce activity.tcl ns-3 file */
../../tools/traceExporter.py --fcd-input=output/fcd.xml --ns2mobility-output=output/activity.tcl

/* Produce config.tcl ns-3 file */
../../tools/traceExporter.py --fcd-input=output/fcd.xml --ns2mobility-output=output/config.tcl
