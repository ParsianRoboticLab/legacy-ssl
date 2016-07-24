#ifndef KHERSDEFENSE_H
#define KHERSDEFENSE_H

#include <plans/plan.h>
#include <gotopoint.h>
#include <robot.h>

#define AGENT_COUNT 10

class KhersDefense : public Plan
{
public:
    KhersDefense();

	void execute();

private:
	std::vector<CRobot*> opp;

	CSkillGotoPoint* gp[AGENT_COUNT];
	CSkillKick* kick;

	bool differ(const std::vector<int>& table1, const std::vector<int>& table2);
};

#endif // KHERSDEFENSE_H
