/* Create net.xml */
netconvert -n scenario3.nod.xml -e scenario3.edg.xml -t scenario3.typ.xml -o output/scenario3.net.xml

/* See net in GUI */
../../src/sumo-gui -n output/scenario3.net.xml

/* See net + rou in GUI */
../../src/sumo-gui -n output/scenario3.net.xml -r scenario3.rou.xml

/* Produce fcd */
sumo -n output/scenario3.net.xml -r scenario3.rou.xml --fcd-output output/fcd.xml

/* Produce mobility.tcl ns-3 file */
../../tools/traceExporter.py --fcd-input=output/fcd.xml --ns2mobility-output=output/mobility.tcl

/* Produce activity.tcl ns-3 file */
../../tools/traceExporter.py --fcd-input=output/fcd.xml --ns2mobility-output=output/activity.tcl

/* Produce config.tcl ns-3 file */
../../tools/traceExporter.py --fcd-input=output/fcd.xml --ns2mobility-output=output/config.tcl
