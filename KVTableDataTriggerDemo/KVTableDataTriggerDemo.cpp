// KVTableDataTriggerDemo.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

int OnTableChanged(etopstatus ops, Schema::Table tab, c_str name)
{
	printf("OnTableChanged type = %d key = %s\n", ops, name);
	return true;
}

struct ITrigger
{
	virtual int OnVirtualTrigger(etopstatus ops, Schema::Table tab, c_str name) = 0;
};

class Trigger : public ITrigger
{
public:
	Trigger() {}
	virtual ~Trigger() {}

	int Register(Schema::Table tab)
	{
		tab.reserve_regc("Virtual Trigger", this, &Trigger::OnVirtualTrigger);
		tab.reservec("Class Trigger");
		tab.regc("Class Trigger", this, &Trigger::OnClassTrigger);
		return true;
	}

	virtual int OnVirtualTrigger(etopstatus ops, Schema::Table tab, c_str name)
	{
		printf("OnVirtualTrigger type = %d key = %s\n", ops, name);
		return true;
	}

private:
	int OnClassTrigger(etopstatus ops, Schema::Table tab, c_str name)
	{
		printf("OnClassTrigger type = %d key = %s\n", ops, name);
		return true;
	}
};

int main()
{
	Schema::Table tab = Schema::CreateTable();
	tab.reg(OnTableChanged);
	tab.setc("trigger1", 0);
	tab.erasec("trigger1");
	tab.setc("trigger1", 2, etc_disable);
	Trigger trgr;
	trgr.Register(tab);
	tab.setc("Virtual Trigger", 2);
	tab.setc("Class Trigger", 3);
	tab.unreg(OnTableChanged);
    return 0;
}

