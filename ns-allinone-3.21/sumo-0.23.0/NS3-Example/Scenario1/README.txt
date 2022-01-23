/* Create net.xml */
netconvert -n scenario1.nod.xml -e scenario1.edg.xml -t scenario1.typ.xml -o output/scenario1.net.xml

/* See net in GUI */
../../src/sumo-gui -n output/scenario1.net.xml

/* See net + rou in GUI */
../../src/sumo-gui -n output/scenario1.net.xml -r scenario1.rou.xml

/* Produce fcd */
sumo -n output/scenario1.net.xml -r scenario1.rou.xml --fcd-output output/fcd.xml

/* Produce mobility.tcl ns-3 file */
../../tools/traceExporter.py --fcd-input=output/fcd.xml --ns2mobility-output=output/mobility.tcl

/* Produce activity.tcl ns-3 file */
../../tools/traceExporter.py --fcd-input=output/fcd.xml --ns2mobility-output=output/activity.tcl

/* Produce config.tcl ns-3 file */
../../tools/traceExporter.py --fcd-input=output/fcd.xml --ns2mobility-output=output/config.tcl
