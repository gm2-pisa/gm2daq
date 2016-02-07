#
# update XML from CSV sources
# (this is a hack, and should have some fancy rules)
#

t1_ver = v0x4015
t1_ldc = v0x00fb
t2_ver = v0x26

all: ../../../amc13/etc/amc13/AMC13XG_T1.xml \
 ../../../amc13/etc/amc13/AMC13XG_T1_DAQLDC.xml \
 ../../../amc13/etc/amc13/AMC13XG_T2.xml

../../../amc13/etc/amc13/AMC13XG_T1.xml: AMC13XG_T1_$(t1_ver).csv
	perl csv_expand_repeat.pl AMC13XG_T1_$(t1_ver).csv temp.csv
	perl csv_to_xml.pl temp.csv ../../../amc13/etc/amc13/AMC13XG_T1.xml

../../../amc13/etc/amc13/AMC13XG_T1_DAQLDC.xml: AMC13XG_T1_$(t1_ldc).csv
	perl csv_expand_repeat.pl AMC13XG_T1_$(t1_ldc).csv temp.csv
	perl csv_to_xml.pl temp.csv ../../../amc13/etc/amc13/AMC13XG_T1_DAQLDC.xml

../../../amc13/etc/amc13/AMC13XG_T2.xml: AMC13XG_T2_$(t2_ver).csv
	perl csv_expand_repeat.pl AMC13XG_T2_$(t2_ver).csv temp.csv
	perl csv_to_xml.pl temp.csv ../../../amc13/etc/amc13/AMC13XG_T2.xml
