#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#define date_t long long
#define BUFSIZE 100
#define MONTHBUF 14
int noNumericDates = 0;
enum dateStates {
	dateStart,
	dateDigit,
	dateYearDash,
	dateYearDashAlphaDash,
	dateYearDashDash,
	dateYearDashDashWs,
	dateYearDashDashT,
	dateDigitDash,
	dateDigitDashAlpha,
	dateDigitDashAlphaDash,
	dateDigitDot,
	dateDigitDotDot,
	dateDigitSlash,
	dateDigitChineseYear,
	dateDigitChineseYearWs,
	dateDigitWs,
	dateDigitWsMoYear,
	dateDigitWsMolong,
	dateAlpha,
	dateAlphaWs,
	dateAlphaWsDigit,
	dateAlphaWsDigitMore,
	dateAlphaWsDigitMoreWs,
	dateAlphaWsDigitMoreWsYear,
	dateAlphaWsMonth,
	dateAlphaWsMonthMore,
	dateAlphaWsMonthSuffix,
	dateAlphaWsMore,
	dateAlphaWsAtTime,
	dateAlphaWsAlpha,
	dateAlphaWsAlphaYearmaybe,
	dateAlphaPeriodWsDigit,
	dateWeekdayComma,
	dateWeekdayAbbrevComma
};

enum timeStates {
	timeIgnore,
	timeStart,
	timeWs,
	timeWsAlpha,
	timeWsAlphaWs,
	timeWsAlphaZoneOffset,
	timeWsAlphaZoneOffsetWs,
	timeWsAlphaZoneOffsetWsYear,
	timeWsAlphaZoneOffsetWsExtra,
	timeWsAMPMMaybe,
	timeWsAMPM,
	timeWsOffset,
	timeWsOffsetWs,
	timeWsOffsetColonAlpha,
	timeWsOffsetColon,
	timeWsYear,
	timeOffset,
	timeOffsetColon,
	timeAlpha,
	timePeriod,
	timePeriodOffset,
	timePeriodOffsetColon,
	timePeriodOffsetColonWs,
	timePeriodWs,
	timePeriodWsAlpha,
	timePeriodWsOffset,
	timePeriodWsOffsetWs,
	timePeriodWsOffsetWsAlpha,
	timePeriodWsOffsetColon,
	timePeriodWsOffsetColonAlpha,
	timeZ,
	timeZDigit
};


static char* months[] = {
	"january",
	"february",
	"march",
	"april",
	"may",
	"june",
	"july",
	"august",
	"september",
	"october",
	"november",
	"december"
};
unsigned int monthlens[] = { 7,8,5,5,3,4,4,6,9,7,8,8 };
char* monthnames[] = {
	"January",
	"February",
	"March",
	"April",
	"May",
	"June",
	"July",
	"August",
	"September",
	"October",
	"November",
	"December"
};
unsigned int daylens[] = { 6,6,7,9,8,6,8 };
char* daynames[] = {
	"Sunday",
	"Monday",
	"Tuesday",
	"Wednesday",
	"Thursday",
	"Friday",
	"Saturday"
};

struct parser {
	//loc              *time.Location
	int len;
	int preferMonthFirst;
	int ambiguousMD;
	unsigned char stateDate;
	unsigned char stateTime;
	char* datestr;
	char datestrbuf[BUFSIZE];
	char fullMonth[MONTHBUF];
	int skip;
	int extra;
	int part1Len;
	int yeari;
	int yearlen;
	char yearbuf[6];
	int moi;
	int molen;
	char mobuf[12];
	int dayi;
	int daylen;
	char daybuf[12];
	char wdaybuf[12];
	int houri;
	int hourlen;
	char hourbuf[6];
	char ampmbuf[3];
	int mini;
	int minlen;
	char minbuf[6];
	int seci;
	int seclen;
	char secbuf[6];
	int usi;
	int uslen;
	char usbuf[12];
	int offseti;
	int offsetlen;
	char offsetbuf[12];
	int tzi;
	int tzlen;
	char tzbuf[30];
	date_t t;
};

#define LEAPOCH (946684800LL + 86400*(31+29))
#define DAYS_PER_400Y (365*400 + 97)
#define DAYS_PER_100Y (365*100 + 24)
#define DAYS_PER_4Y   (365*4   + 1)
int secs_to_tm(long long t, struct tm *tm){
	long long days, secs, years;
	int remdays, remsecs, remyears;
	int qc_cycles, c_cycles, q_cycles;
	int months;
	int wday, yday, leap;
	static const char days_in_month[] = {31,30,31,30,31,31,30,31,30,31,31,29};
	/* Reject date_t values whose year would overflow int */
	if (t < INT_MIN * 31622400LL || t > INT_MAX * 31622400LL)
		return -1;
	secs = t - LEAPOCH;
	days = secs / 86400;
	remsecs = secs % 86400;
	if (remsecs < 0) {
		remsecs += 86400;
		days--;
	}
	wday = (3+days)%7;
	if (wday < 0) wday += 7;
	qc_cycles = days / DAYS_PER_400Y;
	remdays = days % DAYS_PER_400Y;
	if (remdays < 0) {
		remdays += DAYS_PER_400Y;
		qc_cycles--;
	}
	c_cycles = remdays / DAYS_PER_100Y;
	if (c_cycles == 4) c_cycles--;
	remdays -= c_cycles * DAYS_PER_100Y;
	q_cycles = remdays / DAYS_PER_4Y;
	if (q_cycles == 25) q_cycles--;
	remdays -= q_cycles * DAYS_PER_4Y;
	remyears = remdays / 365;
	if (remyears == 4) remyears--;
	remdays -= remyears * 365;
	leap = !remyears && (q_cycles || !c_cycles);
	yday = remdays + 31 + 28 + leap;
	if (yday >= 365+leap) yday -= 365+leap;
	years = remyears + 4*q_cycles + 100*c_cycles + 400LL*qc_cycles;
	for (months=0; days_in_month[months] <= remdays; months++)
		remdays -= days_in_month[months];
	if (months >= 10) {
		months -= 12;
		years++;
	}
	if (years+100 > INT_MAX || years+100 < INT_MIN)
		return -1;
	tm->tm_year = years + 100;
	tm->tm_mon = months + 2;
	tm->tm_mday = remdays + 1;
	tm->tm_wday = wday;
	tm->tm_yday = yday;
	tm->tm_hour = remsecs / 3600;
	tm->tm_min = remsecs / 60 % 60;
	tm->tm_sec = remsecs % 60;
	return 0;
}
struct tm * gmtime64(date_t t){
	static struct tm tmm;
	if (t < 0 && t%1000000) t -= 1000000; //microseconds increment seconds digit when negative
	secs_to_tm(t/1000000, &tmm);
	return &tmm;
}

long long __tm_to_secs(const struct tm *tm);
date_t mktimegm(const struct tm *tm){
	return __tm_to_secs(tm) * 1000000;
}

static void newParser(const char* s, struct parser* p, int stringlen){
	memset(p, 0, sizeof(struct parser));
	p->stateDate = dateStart;
	p->stateTime = timeIgnore;
	p->preferMonthFirst = 1;
	p->datestr = (char*)s;
	p->len = stringlen;
}

static void foundYear(struct parser* p){
	if (p->yearlen == 2) {
		strncpy(p->yearbuf, p->datestr+p->yeari, 2);
		p->yearbuf[p->yearlen] = 0;
	} else if (p->yearlen == 4) {
		strncpy(p->yearbuf, p->datestr+p->yeari, 4);
		p->yearbuf[p->yearlen] = 0;
	}
}
static void foundMonth(struct parser* p){
	if (p->molen == 2) {
		strncpy(p->mobuf, p->datestr+p->moi, 2);
		p->mobuf[p->molen] = 0;
	} else if (p->molen == 1) {
		strncpy(p->mobuf, p->datestr+p->moi, 1);
		p->mobuf[p->molen] = 0;
	}
}
static void foundDay(struct parser* p){
	if (p->daylen == 2) {
		strncpy(p->daybuf, p->datestr+p->dayi, 2);
		p->daybuf[p->daylen] = 0;
	} else if (p->daylen == 1) {
		strncpy(p->daybuf, p->datestr+p->dayi, 1);
		p->daybuf[p->daylen] = 0;
	}
}
static int setMonth(struct parser* p, int i, int len){
	if (len == 0)
		for (; isalpha(p->datestr[i+len]); len++)
			p->mobuf[len] = p->datestr[i+len];
	else
		strncpy(p->mobuf, p->datestr+i, len);
	p->mobuf[len] = 0;
	return len;
}
static void setMinutes(struct parser* p, int i, int len){
	strncpy(p->minbuf, p->datestr+i, len);
	p->minbuf[len] = 0;
}
static void setSeconds(struct parser* p, int i, int len){
	strncpy(p->secbuf, p->datestr+i, len);
	p->secbuf[len] = 0;
}
static void setMicroseconds(struct parser* p, int i, int len){
	len = len > 11 ? 11 : len;
	strncpy(p->usbuf, p->datestr+i, len);
	p->usbuf[len] = 0;
}
static void setDay(struct parser* p, int i, int len){
	strncpy(p->daybuf, p->datestr+i, len);
	p->daybuf[len] = 0;
}
static void setWeekday(struct parser* p, int i, int len){ //not used by mktime()
	strncpy(p->wdaybuf, p->datestr+i, len);
	p->wdaybuf[len] = 0;
}
static void setYear(struct parser* p, int i, int len){
	strncpy(p->yearbuf, p->datestr+i, len);
	p->yearbuf[len] = 0;
}
static void setAmPm(struct parser* p, int i, int len){
	strncpy(p->ampmbuf, p->datestr+i, len);
	p->ampmbuf[len] = 0;
}
static void setTimezone(struct parser* p, int i, int len){
	strncpy(p->tzbuf, p->datestr+i, len);
	p->tzbuf[len] = 0;
}
static void setOffset(struct parser* p, int i, int len){
	strncpy(p->offsetbuf, p->datestr+i, len);
	p->offsetbuf[len] = 0;
}
static void setHour(struct parser* p, int i, int len){
	strncpy(p->hourbuf, p->datestr+i, len);
	p->hourbuf[len] = 0;
}
//copy lowercase month to buffer for month comparison
static void lowerMonth(char* d, const char* s){
	int j;
	for (j=0; j<9 && s[j] && s[j]!=' '; ++j)
		d[j] = tolower(s[j]);
	d[j] = 0;
	//printf("lowermonth %s  ", d);
}
static int isMonthFull(char* s){
	int i;
	for (i=0; i<12; ++i){
		if (!strcmp(s, months[i]))
			return 1;
	}
	return 0;
}
static int nextIs(struct parser* p, int i, char c){
	if (p->len > i+1 && p->datestr[i+1] == c) {
		return 1;
	}
	return 0;
}
static void coalesceDate(struct parser* p, int end) {
	if (p->yeari > 0) {
		if (p->yearlen == 0) {
			p->yearlen = end - p->yeari;
		}
		foundYear(p);
	}
	if (p->moi > 0 && p->molen == 0) {
		p->molen = end - p->moi;
		foundMonth(p);
	}
	if (p->dayi > 0 && p->daylen == 0) {
		p->daylen = end - p->dayi;
		foundDay(p);
	}
}
static void coalesceTime(struct parser* p, int end) {
	// 03:04:05
	// 15:04:05
	// 3:04:05
	// 3:4:5
	// 15:04:05.00
	if (p->houri > 0) {
		if (p->hourlen == 2) {
			setHour(p, p->houri, 2);
		} else if (p->hourlen == 1) {
			setHour(p, p->houri, 1);
		}
	}
	if (p->mini > 0) {
		if (p->minlen == 0) {
			p->minlen = end - p->mini;
		}
		if (p->minlen == 2) {
			setMinutes(p, p->mini, 2);
		} else {
			setMinutes(p, p->mini, 1);
		}
	}
	if (p->seci > 0) {
		if (p->seclen == 0) {
			p->seclen = end - p->seci;
		}
		if (p->seclen == 2) {
			setSeconds(p, p->seci, 2);
		} else {
			setSeconds(p, p->seci, 1);
		}
	}

	if (p->usi > 0) {
		setMicroseconds(p, p->usi, p->uslen);
	}
}
static void setFullMonth(struct parser* p, char* month){
	strcpy(p->mobuf, month);
}
static void trimExtra(struct parser* p){
	if (p->extra > 0) {
		if (p->datestr != p->datestrbuf){
			strncpy(p->datestrbuf, p->datestr, BUFSIZE);
			p->datestr = p->datestrbuf;
		}
		p->datestr[p->extra] = 0;
	}
}
static int isInt(const char* s){
	if (*s == 0) return 0;
	while (*s && isdigit(*s)) ++s;
	return *s == 0;
}

static int parseTime(const char* datestr, struct parser* p, int stringlen){

	newParser(datestr, p, stringlen);
	int len = p->len, i=0, length;
	if (len > 59) return -1;
	char r;
	char buf[BUFSIZE], month[BUFSIZE];

	for (i=0; i<len; ++i){
		r = datestr[i];
		
		switch(p->stateDate){

		case dateStart:
			if (isdigit(r)) {
				p->stateDate = dateDigit;
			} else if (isalpha(r)) {
				p->stateDate = dateAlpha;
			} else {
				return -1;
			}
			break;

		case dateDigit:
			switch (r) {
			case '-':
				// 2006-01-02
				// 2013-Feb-03
				// 13-Feb-03
				// 29-Jun-2016
				if (i == 4) {
					p->stateDate = dateYearDash;
					p->yeari = 0;
					p->yearlen = i;
					p->moi = i + 1;
					setYear(p,0,4);
				} else {
					p->stateDate = dateDigitDash;
				}
				break;

			case '/':
				// 03/31/2005
				// 2014/02/24
				p->stateDate = dateDigitSlash;
				if (i == 4) {
					p->yearlen = i;
					p->moi = i + 1;
					foundYear(p);
				} else {
					p->ambiguousMD = 1;
					if (p->preferMonthFirst) {
						if (p->molen == 0) {
							p->molen = i;
							foundMonth(p);
							p->dayi = i + 1;
						}
					}
				}
				break;

			case '.':
				// 3.31.2014
				// 08.21.71
				// 2014.05
				p->stateDate = dateDigitDot;
				if (i == 4) {
					p->yearlen = i;
					p->moi = i + 1;
					foundYear(p);
				} else {
					p->ambiguousMD = 1;
					p->moi = 0;
					p->molen = i;
					foundMonth(p);
					p->dayi = i + 1;
				}
				break;

			case ' ':
				// 18 January 2018
				// 8 January 2018
				// 8 jan 2018
				// 02 Jan 2018 23:59
				// 02 Jan 2018 23:59:34
				// 12 Feb 2006, 19:17
				// 12 Feb 2006, 19:17:22
				p->stateDate = dateDigitWs;
				p->dayi = 0;
				p->daylen = i;
				break;

			case ',':
				return -1;
			default:
				continue;
			}//inner switch
			p->part1Len = i;
			break;

		case dateYearDash:
			// dateYearDashDashT
			//  2006-01-02T15:04:05Z07:00
			// dateYearDashDashWs
			//  2013-04-01 22:43:22
			// dateYearDashAlphaDash
			//   2013-Feb-03
			switch (r) {
			case '-':
				p->molen = i - p->moi;
				p->dayi = i + 1;
				p->stateDate = dateYearDashDash;
				foundMonth(p);
				break;
			default:
				if (isalpha(r)) {
					p->stateDate = dateYearDashAlphaDash;
				}
			}
			break;

		case dateYearDashDash:
			// dateYearDashDashT
			//  2006-01-02T15:04:05Z07:00
			// dateYearDashDashWs
			//  2013-04-01 22:43:22
			switch (r) {
			case ' ':
				p->daylen = i - p->dayi;
				p->stateDate = dateYearDashDashWs;
				p->stateTime = timeStart;
				foundDay(p);
				goto endIterRunes;
			case 'T':
				p->daylen = i - p->dayi;
				p->stateDate = dateYearDashDashT;
				p->stateTime = timeStart;
				foundDay(p);
				goto endIterRunes;
			}
			break;
		case dateYearDashAlphaDash:
			// 2013-Feb-03
			switch (r) {
			case '-':
				p->molen = i - p->moi;
				setMonth(p,p->moi,3);
				p->dayi = i + 1;
			}
			break;
		case dateDigitDash:
			// 13-Feb-03
			// 29-Jun-2016
			if (isalpha(r)) {
				p->stateDate = dateDigitDashAlpha;
				p->moi = i;
			} else {
				return -1;
			}
			break;
		case dateDigitDashAlpha:
			// 13-Feb-03
			// 28-Feb-03
			// 29-Jun-2016
			switch (r) {
			case '-':
				p->molen = i - p->moi;
				setMonth(p,p->moi,3);
				p->yeari = i + 1;
				p->stateDate = dateDigitDashAlphaDash;
			}
			break;

		case dateDigitDashAlphaDash:
			// 13-Feb-03   ambiguous
			// 28-Feb-03   ambiguous
			// 29-Jun-2016
			switch (r) {
			case ' ':
				// we need to find if this was 4 digits, aka year
				// or 2 digits which makes it ambiguous year/day
				length = i - (p->moi + p->molen + 1);
				if (length == 4) {
					p->yearlen = 4;
					setYear(p,p->yeari,4);
					// We now also know that part1 was the day
					p->dayi = 0;
					p->daylen = p->part1Len;
					foundDay(p);
				} else if (length == 2) {
					// We have no idea if this is
					// yy-mon-dd   OR  dd-mon-yy
					//
					// We are going to ASSUME (bad, bad) that it is dd-mon-yy  which is a horible assumption
					p->ambiguousMD = 1;
					p->yearlen = 2;
					setYear(p,p->yeari,2);
					// We now also know that part1 was the day
					p->dayi = 0;
					p->daylen = p->part1Len;
					foundDay(p);
				}
				p->stateTime = timeStart;
				goto endIterRunes;
			}
			break;

		case dateDigitSlash:
			// 2014/07/10 06:55:38.156283
			// 03/19/2012 10:11:59
			// 04/2/2014 03:00:37
			// 3/1/2012 10:11:59
			// 4/8/2014 22:05
			// 3/1/2014
			// 10/13/2014
			// 01/02/2006
			// 1/2/06

			switch (r) {
			case ' ':
				p->stateTime = timeStart;
				if (p->yearlen == 0) {
					p->yearlen = i - p->yeari;
					foundYear(p);
				} else if (p->daylen == 0) {
					p->daylen = i - p->dayi;
					foundDay(p);
				}
				goto endIterRunes;
			case '/':
				if (p->yearlen > 0) {
					// 2014/07/10 06:55:38.156283
					if (p->molen == 0) {
						p->molen = i - p->moi;
						foundMonth(p);
						p->dayi = i + 1;
					}
				} else if (p->preferMonthFirst) {
					if (p->daylen == 0) {
						p->daylen = i - p->dayi;
						foundDay(p);
						p->yeari = i + 1;
					}
				}
			}
			break;

		case dateDigitWs:
			// 18 January 2018
			// 8 January 2018
			// 8 jan 2018
			// 1 jan 18
			// 02 Jan 2018 23:59
			// 02 Jan 2018 23:59:34
			// 12 Feb 2006, 19:17
			// 12 Feb 2006, 19:17:22
			switch (r) {
			case ' ':
				p->yeari = i + 1;
				//p->yearlen = 4
				p->dayi = 0;
				p->daylen = p->part1Len;
				foundDay(p);
				p->stateTime = timeStart;
				if (i > p->daylen+4) { // was strlen(" Sep") // November etc
					// If len greather than space + 3 it must be full month
					p->stateDate = dateDigitWsMolong;
				} else {
					// If len=3, the might be Feb or May?  Ie ambigous abbreviated but
					// we can parse may with either.  BUT, that means the
					// format may not be correct?
					// mo := strings.ToLower(datestr[p->daylen+1 : i])
					p->moi = p->daylen + 1;
					p->molen = i - p->moi;
					setMonth(p,p->moi,3);
					p->stateDate = dateDigitWsMoYear;
				}
				break;
			}
			break;

		case dateDigitWsMoYear:
			// 8 jan 2018
			// 02 Jan 2018 23:59
			// 02 Jan 2018 23:59:34
			// 12 Feb 2006, 19:17
			// 12 Feb 2006, 19:17:22
			switch (r) {
			case ',':
				p->yearlen = i - p->yeari;
				foundYear(p);
				i++;
				goto endIterRunes;
			case ' ':
				p->yearlen = i - p->yeari;
				foundYear(p);
				goto endIterRunes;
			}
			break;
		case dateDigitWsMolong:
			break;
			// 18 January 2018
			// 8 January 2018

		case dateDigitChineseYear:
			// dateDigitChineseYear
			//   2014年04月08日
			//               weekday  %Y年%m月%e日 %A %I:%M %p
			// 2013年07月18日 星期四 10:27 上午
			if (r == ' ') {
				p->stateDate = dateDigitChineseYearWs;
				break;
			}
			break;

		case dateDigitDot:
			// This is the 2nd period
			// 3.31.2014
			// 08.21.71
			// 2014.05
			// 2018.09.30
			if (r == '.') {
				if (p->moi == 0) {
					// 3.31.2014
					p->daylen = i - p->dayi;
					p->yeari = i + 1;
					foundDay(p);
					p->stateDate = dateDigitDotDot;
				} else {
					// 2018.09.30
					//p->molen = 2
					p->molen = i - p->moi;
					p->dayi = i + 1;
					foundMonth(p);
					p->stateDate = dateDigitDotDot;
				}
			}
		case dateDigitDotDot:
			// iterate all the way through
			break;

		case dateAlpha:
			// dateAlphaWS
			//  Mon Jan _2 15:04:05 2006
			//  Mon Jan _2 15:04:05 MST 2006
			//  Mon Jan 02 15:04:05 -0700 2006
			//  Mon Aug 10 15:44:11 UTC+0100 2015
			//  Fri Jul 03 2015 18:04:07 GMT+0100 (GMT Daylight Time)
			//  dateAlphaWSDigit
			//    May 8, 2009 5:57:51 PM
			//    oct 1, 1970
			//  dateAlphaWsMonth
			//    April 8, 2009
			//  dateAlphaWsMore
			//    dateAlphaWsAtTime
			//      January 02, 2006 at 3:04pm MST-07
			//
			//  dateAlphaPeriodWsDigit
			//    oct. 1, 1970
			// dateWeekdayComma
			//   Monday, 02 Jan 2006 15:04:05 MST
			//   Monday, 02-Jan-06 15:04:05 MST
			//   Monday, 02 Jan 2006 15:04:05 -0700
			//   Monday, 02 Jan 2006 15:04:05 +0100
			// dateWeekdayAbbrevComma
			//   Mon, 02 Jan 2006 15:04:05 MST
			//   Mon, 02 Jan 2006 15:04:05 -0700
			//   Thu, 13 Jul 2017 08:58:40 +0100
			//   Tue, 11 Jul 2017 16:28:13 +0200 (CEST)
			//   Mon, 02-Jan-06 15:04:05 MST
			switch (r) {
			case ' ':
				//      X
				// April 8, 2009
				if (i > 3) {
					// Check to see if the alpha is name of month?  or Day?
					lowerMonth(month, datestr);
					if (isMonthFull(month)) {
						strcpy(p->mobuf, month);
						// len(" 31, 2018")   = 9
						if (p->len - i < 10) {
							// April 8, 2009
							p->stateDate = dateAlphaWsMonth;
						} else {
							p->stateDate = dateAlphaWsMore;
						}
						p->dayi = i + 1;
						break;
					}

				} else {
					// This is possibly ambiguous?  May will parse as either though.
					// So, it could return in-correct format.
					// May 05, 2005, 05:05:05
					// May 05 2005, 05:05:05
					// Jul 05, 2005, 05:05:05
					p->stateDate = dateAlphaWs;
				}
				break;

			case ',':
				// Mon, 02 Jan 2006
				// p->moi = 0
				// p->molen = i
				if (i == 3) {
					p->stateDate = dateWeekdayAbbrevComma;
					//setWeekday(p, 0, 3);
				} else {
					p->stateDate = dateWeekdayComma;
					p->skip = i + 2;
					i++;
					// TODO:  lets just make this "skip" as we don't need
					// the mon, monday, they are all superfelous and not needed
					// just lay down the skip, no need to fill and then skip
				}
				break;
			case '.':
				// sept. 28, 2017
				// jan. 28, 2017
				p->stateDate = dateAlphaPeriodWsDigit;
				if (i == 3) {
					p->molen = i;
					setMonth(p, 0, 3);
				} else if (i == 4) {
					// gross
					//datestr = datestr[0:i-1] + datestr[i:];
					//return parseTime(datestr, loc);
					return -1; //locations not implemented yet
				} else {
					return -1;
				}
				break;
			}//inner switch
			break;

		case dateAlphaWs:
			// dateAlphaWsAlpha
			//   Mon Jan _2 15:04:05 2006
			//   Mon Jan _2 15:04:05 MST 2006
			//   Mon Jan 02 15:04:05 -0700 2006
			//   Fri Jul 03 2015 18:04:07 GMT+0100 (GMT Daylight Time)
			//   Mon Aug 10 15:44:11 UTC+0100 2015
			//  dateAlphaWsDigit
			//    May 8, 2009 5:57:51 PM
			//    May 8 2009 5:57:51 PM
			//    oct 1, 1970
			//    oct 7, '70
			if (isalpha(r)) {
				//setWeekday(p, 0, 3);
				p->stateDate = dateAlphaWsAlpha;
				setMonth(p, i, 3);
			} else if (isdigit(r)) {
				setMonth(p, 0, 3);
				p->stateDate = dateAlphaWsDigit;
				p->dayi = i;
			}
			break;

		case dateAlphaWsDigit:
			// May 8, 2009 5:57:51 PM
			// May 8 2009 5:57:51 PM
			// oct 1, 1970
			// oct 7, '70
			// oct. 7, 1970
			if (r == ',') {
				p->daylen = i - p->dayi;
				foundDay(p);
				p->stateDate = dateAlphaWsDigitMore;
			} else if (r == ' ') {
				p->daylen = i - p->dayi;
				foundDay(p);
				p->yeari = i + 1;
				p->stateDate = dateAlphaWsDigitMoreWs;
			} else if (isalpha(r)) {
				p->stateDate = dateAlphaWsMonthSuffix;
				i--;
			}
			break;
		case dateAlphaWsDigitMore:
			//       x
			// May 8, 2009 5:57:51 PM
			// May 05, 2005, 05:05:05
			// May 05 2005, 05:05:05
			// oct 1, 1970
			// oct 7, '70
			if (r == ' ') {
				p->yeari = i + 1;
				p->stateDate = dateAlphaWsDigitMoreWs;
			}
			break;

		case dateAlphaWsDigitMoreWs:
			//            x
			// May 8, 2009 5:57:51 PM
			// May 05, 2005, 05:05:05
			// oct 1, 1970
			// oct 7, '70
			switch (r) {
			case '\'':
				p->yeari = i + 1;
				break;
			case ' ':
			case ',':
				//            x
				// May 8, 2009 5:57:51 PM
				//            x
				// May 8, 2009, 5:57:51 PM
				p->stateDate = dateAlphaWsDigitMoreWsYear;
				p->yearlen = i - p->yeari;
				foundYear(p);
				p->stateTime = timeStart;
				goto endIterRunes;
			}
			break;

		case dateAlphaWsAlpha:
			// Mon Jan _2 15:04:05 2006
			// Mon Jan 02 15:04:05 -0700 2006
			// Mon Jan _2 15:04:05 MST 2006
			// Mon Aug 10 15:44:11 UTC+0100 2015
			// Fri Jul 03 2015 18:04:07 GMT+0100 (GMT Daylight Time)
			if (r == ' ') {
				if (p->dayi > 0) {
					p->daylen = i - p->dayi;
					foundDay(p);
					p->yeari = i + 1;
					p->stateDate = dateAlphaWsAlphaYearmaybe;
					p->stateTime = timeStart;
				}
			} else if (isdigit(r)) {
				if (p->dayi == 0) {;
					p->dayi = i;
				}
			}
			break;

		case dateAlphaWsAlphaYearmaybe:
			//            x
			// Mon Jan _2 15:04:05 2006
			// Fri Jul 03 2015 18:04:07 GMT+0100 (GMT Daylight Time)
			if (r == ':') {
				i = i - 3;
				p->stateDate = dateAlphaWsAlpha;
				p->yeari = 0;
				goto endIterRunes;
			} else if (r == ' ') {
				// must be year format, not 15:04
				p->yearlen = i - p->yeari;
				foundYear(p);
				goto endIterRunes;
			}
			break;

		case dateAlphaWsMonth:
			// April 8, 2009
			// April 8 2009
			switch (r) {
			case ',':
			case ' ':
				//       x
				// June 8, 2009
				//       x
				// June 8 2009
				if (p->daylen == 0) {
					p->daylen = i - p->dayi;
					foundDay(p);
				}
				break;
			case 's':
			case 'S':
			case 'r':
			case 'R':
			case 't':
			case 'T':
			case 'n':
			case 'N':
				// st, rd, nd, st
				i--;
				p->stateDate = dateAlphaWsMonthSuffix;
				break;
			default:
				if (p->daylen > 0 && p->yeari == 0) {
					p->yeari = i;
				}
			}
			break;

		case dateAlphaWsMonthMore:
			//                  X
			// January 02, 2006, 15:04:05
			// January 02 2006, 15:04:05
			// January 02, 2006 15:04:05
			// January 02 2006 15:04:05
			switch (r) {
			case ',':
				p->yearlen = i - p->yeari;
				foundYear(p);
				p->stateTime = timeStart;
				i++;
				goto endIterRunes;
			case ' ':
				p->yearlen = i - p->yeari;
				foundYear(p);
				p->stateTime = timeStart;
				goto endIterRunes;
			}
			break;

		case dateAlphaWsMonthSuffix:
			//        x
			// April 8th, 2009
			// April 8th 2009
			memset(buf,0,BUFSIZE);
			switch (r) {
			case 't':
			case 'T':
				if (nextIs(p, i, 'h') || nextIs(p, i, 'H')) {
					if (len > i+2) {
						strncpy(buf, datestr, i);
						strncpy(buf+i, datestr+i+2, BUFSIZE-i-2);
						return parseTime(buf, p, p->len-2);
					}
				}
				break;
			case 'n':
			case 'N':
				if (nextIs(p, i, 'd') || nextIs(p, i, 'D')) {
					if (len > i+2) {
						strncpy(buf, datestr, i);
						strncpy(buf+i, datestr+i+2, BUFSIZE-i-2);
						return parseTime(buf, p, p->len-2);
					}
				}
				break;
			case 's':
			case 'S':
				if (nextIs(p, i, 't') || nextIs(p, i, 'T')) {
					if (len > i+2) {
						strncpy(buf, datestr, i);
						strncpy(buf+i, datestr+i+2, BUFSIZE-i-2);
						return parseTime(buf, p, p->len-2);
					}
				}
				break;
			case 'r':
			case 'R':
				if (nextIs(p, i, 'd') || nextIs(p, i, 'D')) {
					if (len > i+2) {
						strncpy(buf, datestr, i);
						strncpy(buf+i, datestr+i+2, BUFSIZE-i-2);
						return parseTime(buf, p, p->len-2);
					}
				}
			}
			break;

		case dateAlphaWsMore:
			// January 02, 2006, 15:04:05
			// January 02 2006, 15:04:05
			// January 2nd, 2006, 15:04:05
			// January 2nd 2006, 15:04:05
			// September 17, 2012 at 5:00pm UTC-05
			if (r == ','){
				//           x
				// January 02, 2006, 15:04:05
				if (nextIs(p, i, ' ')) {
					p->daylen = i - p->dayi;
					foundDay(p);
					p->yeari = i + 2;
					p->stateDate = dateAlphaWsMonthMore;
					i++;
				}
			} else if (r == ' ') {
				//           x
				// January 02 2006, 15:04:05
				p->daylen = i - p->dayi;
				foundDay(p);
				p->yeari = i + 1;
				p->stateDate = dateAlphaWsMonthMore;
			} else if (isdigit(r)) {
				//         XX
				// January 02, 2006, 15:04:05
				continue;
			} else if (isalpha(r)) {
				//          X
				// January 2nd, 2006, 15:04:05
				p->daylen = i - p->dayi;
				foundDay(p);
				p->stateDate = dateAlphaWsMonthSuffix;
				i--;
			}
			break;

		case dateAlphaPeriodWsDigit:
			//    oct. 7, '70
			if (r == ' '){
				// continue
			} else if (isdigit(r)) {
				p->stateDate = dateAlphaWsDigit;
				p->dayi = i;
			} else {
				return -1;
			}
			break;
		case dateWeekdayComma:
			// Monday, 02 Jan 2006 15:04:05 MST
			// Monday, 02 Jan 2006 15:04:05 -0700
			// Monday, 02 Jan 2006 15:04:05 +0100
			// Monday, 02-Jan-06 15:04:05 MST
			if (p->dayi == 0) {
				p->dayi = i;
			}
			switch (r) {
			case '-':
			case ' ':
				if (p->moi == 0) {
					p->moi = i + 1;
					p->daylen = i - p->dayi;
					foundDay(p);
				} else if (p->yeari == 0) {
					p->yeari = i + 1;
					p->molen = i - p->moi;
					setMonth(p, p->moi, 3);
				} else {
					p->stateTime = timeStart;
					goto endIterRunes;
				}
			}
			break;
		case dateWeekdayAbbrevComma:
			// Mon, 02 Jan 2006 15:04:05 MST
			// Mon, 02 Jan 2006 15:04:05 -0700
			// Thu, 13 Jul 2017 08:58:40 +0100
			// Thu, 4 Jan 2018 17:53:36 +0000
			// Tue, 11 Jul 2017 16:28:13 +0200 (CEST)
			// Mon, 02-Jan-06 15:04:05 MST
			switch (r) {
			case ' ':
			case '-':
				if (p->dayi == 0) {
					p->dayi = i + 1;
				} else if (p->moi == 0) {
					p->daylen = i - p->dayi;
					foundDay(p);
					p->moi = i + 1;
				} else if (p->yeari == 0) {
					p->molen = i - p->moi;
					setMonth(p, p->moi, 3);
					p->yeari = i + 1;
				} else {
					p->yearlen = i - p->yeari;
					foundYear(p);
					p->stateTime = timeStart;
					goto endIterRunes;
				}
			}
			break;

		default:
			goto endIterRunes;
		} //outer switch
	} //for
	endIterRunes:
	coalesceDate(p,i);
	if (p->stateTime == timeStart) {
		// increment first one, since the i++ occurs at end of loop
		if (i < p->len) i++;
		// ensure we skip any whitespace prefix
		for (; i < len; i++) {
			r = datestr[i];
			if (r != ' ') break;
		}
		for (; i < len; i++) {
			r = datestr[i];

			switch (p->stateTime) {
			case timeStart:
				// 22:43:22
				// 22:43
				// timeComma
				//   08:20:13,787
				// timeWs
				//   05:24:37 PM
				//   06:20:00 UTC
				//   06:20:00 UTC-05
				//   00:12:00 +0000 UTC
				//   22:18:00 +0000 UTC m=+0.000000001
				//   15:04:05 -0700
				//   15:04:05 -07:00
				//   15:04:05 2008
				// timeOffset
				//   03:21:51+00:00
				//   19:55:00+0100
				// timePeriod
				//   17:24:37.3186369
				//   00:07:31.945167
				//   18:31:59.257000000
				//   00:00:00.000
				//   timePeriodOffset
				//     19:55:00.799+0100
				//     timePeriodOffsetColon
				//       15:04:05.999-07:00
				//   timePeriodWs
				//     timePeriodWsOffset
				//       00:07:31.945167 +0000
				//       00:00:00.000 +0000
				//     timePeriodWsOffsetAlpha
				//       00:07:31.945167 +0000 UTC
				//       22:18:00.001 +0000 UTC m=+0.000000001
				//       00:00:00.000 +0000 UTC
				//     timePeriodWsAlpha
				//       06:20:00.000 UTC
				if (p->houri == 0) {
					p->houri = i;
				}
				switch (r) {
				case ',':
					// hm, lets just swap out comma for period.  for some reason go
					// won't parse it.
					// 2014-05-11 08:20:13,787
					strcpy(buf, datestr);
					buf[i] = '.';
					return parseTime(buf, p, p->len);
				case '+':
				case '-':
					//   03:21:51+00:00
					p->stateTime = timeOffset;
					if (p->seci == 0) {
						// 22:18+0530
						p->minlen = i - p->mini;
					} else {
						p->seclen = i - p->seci;
					}
					p->offseti = i;
					break;
				case '.':
					p->stateTime = timePeriod;
					p->seclen = i - p->seci;
					p->usi = i + 1;
					break;
				case 'Z':
					p->stateTime = timeZ;
					if (p->seci == 0) {
						p->minlen = i - p->mini;
					} else {
						p->seclen = i - p->seci;
					}
					break;
				case 'A':
				case 'a':
					if (nextIs(p, i, 't') || nextIs(p, i, 'T')) {
						//                    x
						// September 17, 2012 at 5:00pm UTC-05
						i++; // skip t
						if (nextIs(p, i, ' ')) {
							//                      x
							// September 17, 2012 at 5:00pm UTC-05
							i++;         // skip '
							p->houri = 0; // reset hour
						}
					} else {
						if (r == 'a' && nextIs(p, i, 'm')){
							coalesceTime(p, i);
							setAmPm(p, i, 2);
						} else if (r == 'A' && nextIs(p, i, 'M')){
							coalesceTime(p, i);
							setAmPm(p, i, 2);
						}
					}
					break;

				case 'p':
				case 'P':
					// Could be AM/PM
					if (r == 'p' && nextIs(p, i, 'm')){
						coalesceTime(p, i);
						setAmPm(p, i, 2);
					} else if (r == 'P' && nextIs(p, i, 'M')){
						coalesceTime(p, i);
						setAmPm(p, i, 2);
					}
					break;
				case ' ':
					coalesceTime(p, i);
					p->stateTime = timeWs;
					break;
				case ':':
					if (p->mini == 0) {
						p->mini = i + 1;
						p->hourlen = i - p->houri;
					} else if (p->seci == 0) {
						p->seci = i + 1;
						p->minlen = i - p->mini;
					}
				}//inner switch
				break;
			case timeOffset:
				// 19:55:00+0100
				// timeOffsetColon
				//   15:04:05+07:00
				//   15:04:05-07:00
				if (r == ':') { p->stateTime = timeOffsetColon; }
				break;
			case timeWs:
				// timeWsAlpha
				//   06:20:00 UTC
				//   06:20:00 UTC-05
				//   15:44:11 UTC+0100 2015
				//   18:04:07 GMT+0100 (GMT Daylight Time)
				//   17:57:51 MST 2009
				//   timeWsAMPMMaybe
				//     05:24:37 PM
				// timeWsOffset
				//   15:04:05 -0700
				//   00:12:00 +0000 UTC
				//   timeWsOffsetColon
				//     15:04:05 -07:00
				//     17:57:51 -0700 2009
				//     timeWsOffsetColonAlpha
				//       00:12:00 +00:00 UTC
				// timeWsYear
				//     00:12:00 2008
				// timeZ
				//   15:04:05.99Z
				switch (r) {
				case 'P':
				case 'A':
					// Could be AM/PM or could be PST or similar
					p->tzi = i;
					p->stateTime = timeWsAMPMMaybe;
					break;
				case '-':
				case '+':
					p->offseti = i;
					p->stateTime = timeWsOffset;
					break;
				default:
					if (isalpha(r)) {
						// 06:20:00 UTC
						// 06:20:00 UTC-05
						// 15:44:11 UTC+0100 2015
						// 17:57:51 MST 2009
						p->tzi = i;
						p->stateTime = timeWsAlpha;
						//break iterTimeRunes
					} else if (isdigit(r)) {
						// 00:12:00 2008
						p->stateTime = timeWsYear;
						p->yeari = i;
					}
				}
				break;
			case timeWsAlpha:
				// 06:20:00 UTC
				// 06:20:00 UTC-05
				// timeWsAlphaWs
				//   17:57:51 MST 2009
				// timeWsAlphaZoneOffset
				// timeWsAlphaZoneOffsetWs
				//   timeWsAlphaZoneOffsetWsExtra
				//     18:04:07 GMT+0100 (GMT Daylight Time)
				//   timeWsAlphaZoneOffsetWsYear
				//     15:44:11 UTC+0100 2015
				switch (r) {
				case '+':
				case '-':
					p->tzlen = i - p->tzi;
					if (p->tzlen == 4) {
						setTimezone(p, p->tzi+1, 3);
					} else if (p->tzlen == 3) {
						setTimezone(p, p->tzi, 3);
					}
					p->stateTime = timeWsAlphaZoneOffset;
					p->offseti = i;
					break;
				case ' ':
					// 17:57:51 MST 2009
					p->tzlen = i - p->tzi;
					if (p->tzlen == 4) {
						setTimezone(p, p->tzi+1, 3);
					} else if (p->tzlen == 3) {
						setTimezone(p, p->tzi, 3);
					}
					p->stateTime = timeWsAlphaWs;
					p->yeari = i + 1;
					break;
				}
			case timeWsAlphaWs:
				//   17:57:51 MST 2009
				break;

			case timeWsAlphaZoneOffset:
				// 06:20:00 UTC-05
				// timeWsAlphaZoneOffset
				// timeWsAlphaZoneOffsetWs
				//   timeWsAlphaZoneOffsetWsExtra
				//     18:04:07 GMT+0100 (GMT Daylight Time)
				//   timeWsAlphaZoneOffsetWsYear
				//     15:44:11 UTC+0100 2015
				switch (r) {
				case ' ':
					setOffset(p, p->offseti, 5);
					p->yeari = i + 1;
					p->stateTime = timeWsAlphaZoneOffsetWs;
				}
				break;
			case timeWsAlphaZoneOffsetWs:
				// timeWsAlphaZoneOffsetWs
				//   timeWsAlphaZoneOffsetWsExtra
				//     18:04:07 GMT+0100 (GMT Daylight Time)
				//   timeWsAlphaZoneOffsetWsYear
				//     15:44:11 UTC+0100 2015
				if (isdigit(r)) {
					p->stateTime = timeWsAlphaZoneOffsetWsYear;
				} else {
					p->extra = i - 1;
					p->stateTime = timeWsAlphaZoneOffsetWsExtra;
				}
				break;
			case timeWsAlphaZoneOffsetWsYear:
				// 15:44:11 UTC+0100 2015
				if (isdigit(r)) {
					p->yearlen = i - p->yeari + 1;
					if (p->yearlen == 4) {
						foundYear(p);
					}
				}
				break;
			case timeWsAMPMMaybe:
				// timeWsAMPMMaybe
				//   timeWsAMPM
				//     05:24:37 PM
				//   timeWsAlpha
				//     00:12:00 PST
				//     15:44:11 UTC+0100 2015
				if (r == 'M') {
					//return parse("2006-01-02 03:04:05 PM", datestr, loc)
					p->stateTime = timeWsAMPM;
					setAmPm(p, i-1, 2);
					if (p->hourlen == 2) {
						setHour(p, p->houri, 2);
					} else if (p->hourlen == 1) {
						setHour(p, p->houri, 1);
					}
				} else {
					p->stateTime = timeWsAlpha;
				}
				break;

			case timeWsOffset:
				// timeWsOffset
				//   15:04:05 -0700
				//   timeWsOffsetWsOffset
				//     17:57:51 -0700 -07
				//   timeWsOffsetWs
				//     17:57:51 -0700 2009
				//     00:12:00 +0000 UTC
				//   timeWsOffsetColon
				//     15:04:05 -07:00
				//     timeWsOffsetColonAlpha
				//       00:12:00 +00:00 UTC
				switch (r) {
				case ':':
					p->stateTime = timeWsOffsetColon;
					break;
				case ' ':
					setOffset(p, p->offseti, 5);
					p->yeari = i + 1;
					p->stateTime = timeWsOffsetWs;
					break;
				}
				break;
			case timeWsOffsetWs:
				// 17:57:51 -0700 2009
				// 00:12:00 +0000 UTC
				// 22:18:00.001 +0000 UTC m=+0.000000001
				// w Extra
				//   17:57:51 -0700 -07
				switch (r) {
				case '=':
					if (datestr[i-1] == 'm') {
						p->extra = i - 2;
						trimExtra(p);
						break;
					}
					break;
				case '+':
				case '-':
					// This really doesn't seem valid, but for some reason when round-tripping a go date
					// their is an extra +03 printed out.  seems like go bug to me, but, parsing anyway.
					// 00:00:00 +0300 +03
					// 00:00:00 +0300 +0300
					p->extra = i - 1;
					p->stateTime = timeWsOffset;
					trimExtra(p);
					break;
				default:
					if (isdigit(r)){
						p->yearlen = i - p->yeari + 1;
						if (p->yearlen == 4) {
							foundYear(p);
						}
					} else if (isalpha(r)){
						if (p->tzi == 0) {
							p->tzi = i;
						}
					}
					break;
				}
				break;

			case timeWsOffsetColon:
				// timeWsOffsetColon
				//   15:04:05 -07:00
				//   timeWsOffsetColonAlpha
				//     2015-02-18 00:12:00 +00:00 UTC
				if (isalpha(r)) {
					// 2015-02-18 00:12:00 +00:00 UTC
					p->stateTime = timeWsOffsetColonAlpha;
					goto endIterTimeRunes;
				}
				break;
			case timePeriod:
				// 15:04:05.999999999+07:00
				// 15:04:05.999999999-07:00
				// 15:04:05.999999+07:00
				// 15:04:05.999999-07:00
				// 15:04:05.999+07:00
				// 15:04:05.999-07:00
				// timePeriod
				//   17:24:37.3186369
				//   00:07:31.945167
				//   18:31:59.257000000
				//   00:00:00.000
				//   timePeriodOffset
				//     19:55:00.799+0100
				//     timePeriodOffsetColon
				//       15:04:05.999-07:00
				//   timePeriodWs
				//     timePeriodWsOffset
				//       00:07:31.945167 +0000
				//       00:00:00.000 +0000
				//       With Extra
				//         00:00:00.000 +0300 +03
				//     timePeriodWsOffsetAlpha
				//       00:07:31.945167 +0000 UTC
				//       00:00:00.000 +0000 UTC
				//       22:18:00.001 +0000 UTC m=+0.000000001
				//     timePeriodWsAlpha
				//       06:20:00.000 UTC
				switch (r) {
				case ' ':
					p->uslen = i - p->usi;
					p->stateTime = timePeriodWs;
					break;
				case '+':
				case '-':
					// This really shouldn't happen
					p->uslen = i - p->usi;
					p->offseti = i;
					p->stateTime = timePeriodOffset;
					break;
				default:
					if (isalpha(r)) {
						// 06:20:00.000 UTC
						p->uslen = i - p->usi;
						p->stateTime = timePeriodWsAlpha;
					}
					break;
				}
				break;
			case timePeriodOffset:
				// timePeriodOffset
				//   19:55:00.799+0100
				//   timePeriodOffsetColon
				//     15:04:05.999-07:00
				//     13:31:51.999-07:00 MST
				if (r == ':') {
					p->stateTime = timePeriodOffsetColon;
				}
				break;
			case timePeriodOffsetColon:
				// timePeriodOffset
				//   timePeriodOffsetColon
				//     15:04:05.999-07:00
				//     13:31:51.999 -07:00 MST
				switch (r) {
				case ' ':
					setOffset(p, p->offseti, 6);
					p->stateTime = timePeriodOffsetColonWs;
					p->tzi = i + 1;
				}
				break;
			case timePeriodOffsetColonWs:
				// continue
				break;
			case timePeriodWs:
				// timePeriodWs
				//   timePeriodWsOffset
				//     00:07:31.945167 +0000
				//     00:00:00.000 +0000
				//   timePeriodWsOffsetAlpha
				//     00:07:31.945167 +0000 UTC
				//     00:00:00.000 +0000 UTC
				//   timePeriodWsOffsetColon
				//     13:31:51.999 -07:00 MST
				//   timePeriodWsAlpha
				//     06:20:00.000 UTC
				if (p->offseti == 0) {
					p->offseti = i;
				}
				switch (r) {
				case '+':
				case '-':
					p->uslen = i - p->usi - 1;
					p->stateTime = timePeriodWsOffset;
					break;
				default:
					if (isalpha(r)) {
						//     00:07:31.945167 +0000 UTC
						//     00:00:00.000 +0000 UTC
						p->stateTime = timePeriodWsOffsetWsAlpha;
						goto endIterTimeRunes;
					}
				}
				break;

			case timePeriodWsOffset:
				// timePeriodWs
				//   timePeriodWsOffset
				//     00:07:31.945167 +0000
				//     00:00:00.000 +0000
				//     With Extra
				//       00:00:00.000 +0300 +03
				//   timePeriodWsOffsetAlpha
				//     00:07:31.945167 +0000 UTC
				//     00:00:00.000 +0000 UTC
				//     03:02:00.001 +0300 MSK m=+0.000000001
				//   timePeriodWsOffsetColon
				//     13:31:51.999 -07:00 MST
				//   timePeriodWsAlpha
				//     06:20:00.000 UTC
				switch (r) {
				case ':':
					p->stateTime = timePeriodWsOffsetColon;
					break;
				case ' ':
					setOffset(p, p->offseti, 5);
					break;
				case '-':
				case '+':
					// This really doesn't seem valid, but for some reason when round-tripping a go date
					// their is an extra +03 printed out.  seems like go bug to me, but, parsing anyway.
					// 00:00:00.000 +0300 +03
					// 00:00:00.000 +0300 +0300
					p->extra = i - 1;
					trimExtra(p);
					break;
				default:
					if (isalpha(r)) {
						// 00:07:31.945167 +0000 UTC
						// 00:00:00.000 +0000 UTC
						// 03:02:00.001 +0300 MSK m=+0.000000001
						p->stateTime = timePeriodWsOffsetWsAlpha;
					}
					break;
				}
				break;
			case timePeriodWsOffsetWsAlpha:
				// 03:02:00.001 +0300 MSK m=+0.000000001
				// eff you golang
				if (r == '=' && datestr[i-1] == 'm') {
					p->extra = i - 2;
					trimExtra(p);
					break;
				}
				break;

			case timePeriodWsOffsetColon:
				// 13:31:51.999 -07:00 MST
				switch (r) {
				case ' ':
					setOffset(p, p->offseti, 6);
					break;
				default:
					if (isalpha(r)) {
						// 13:31:51.999 -07:00 MST
						p->tzi = i;
						p->stateTime = timePeriodWsOffsetColonAlpha;
					}
				}
				break;
			case timePeriodWsOffsetColonAlpha:
				// continue
				break;
			case timeZ:
				// timeZ
				//   15:04:05.99Z
				// With a time-zone at end after Z
				// 2006-01-02T15:04:05.999999999Z07:00
				// 2006-01-02T15:04:05Z07:00
				// RFC3339     = "2006-01-02T15:04:05Z07:00"
				// RFC3339Nano = "2006-01-02T15:04:05.999999999Z07:00"
				if (isdigit(r)) {
					p->stateTime = timeZDigit;
				}
				break;

			}//outer switch
		}//for
		endIterTimeRunes:

		switch (p->stateTime) {
		case timeWsAlphaWs:
			p->yearlen = i - p->yeari;
			foundYear(p);
			break;
		case timeWsYear:
			p->yearlen = i - p->yeari;
			foundYear(p);
			break;
		case timeWsAlphaZoneOffsetWsExtra:
			trimExtra(p);
			break;
		case timeWsAlphaZoneOffset:
			// 06:20:00 UTC-05
			if (i-p->offseti < 4) {
				setOffset(p, p->offseti, 3);
			} else {
				setOffset(p, p->offseti, 5);
			}
			break;

		case timePeriod:
			p->uslen = i - p->usi;
			break;
		case timeOffset:
			// 19:55:00+0100
			setOffset(p, p->offseti, 5);
			break;
		case timeWsOffset:
			setOffset(p, p->offseti, 5);
			break;
		case timeWsOffsetWs:
			// 17:57:51 -0700 2009
			// 00:12:00 +0000 UTC
			break;
		case timeWsOffsetColon:
			// 17:57:51 -07:00
			setOffset(p, p->offseti, 6);
			break;
		case timeOffsetColon:
			// 15:04:05+07:00
			setOffset(p, p->offseti, 6);
			break;
		case timePeriodOffset:
			// 19:55:00.799+0100
			setOffset(p, p->offseti, 5);
			break;
		case timePeriodOffsetColon:
			setOffset(p, p->offseti, 6);
			break;
		case timePeriodWsOffsetColonAlpha:
			p->tzlen = i - p->tzi;
			switch (p->tzlen) {
			case 3:
				setTimezone(p, p->tzi, 3);
				break;
			case 4:
				setTimezone(p, p->tzi, 3);
				break;
			}
			break;
		case timePeriodWsOffset:
			setOffset(p, p->offseti, 5);
			break;
		}
		coalesceTime(p, i);
	}// time if

	date_t dt = 0;
	switch (p->stateDate) {
	case dateDigit:
		if (noNumericDates)
			break;
		// unixy timestamps ish
		//  example              ct type
		//  1499979655583057426  19 nanoseconds
		//  1499979795437000     16 micro-seconds
		//  20180722105203       14 yyyyMMddhhmmss
		//  1499979795437        13 milliseconds
		//  1332151919           10 seconds
		//  20140601             8  yyyymmdd
		//  2014                 4  yyyy
		// nano-seconds
		if (len == 19) {
            #if INTPTR_MAX == INT64_MAX
			if (isInt(datestr)) {
				long long  nanoSecs = strtoll(datestr, NULL, 10);
				dt = nanoSecs / 1000;
			}
            #endif
		// micro-seconds
		} else if (len == 16) {
            #if INTPTR_MAX == INT64_MAX
			if (isInt(datestr)) {
				dt = strtoll(datestr, NULL, 10);
			}
            #endif
		// yyyyMMddhhmmss
		} else if (len == 14) {
			setYear(p, 0, 4);
			setMonth(p, 4, 2);
			setDay(p, 6, 2);
			setHour(p, 8, 2);
			setMinutes(p, 10, 2);
			setSeconds(p, 12, 2);
			return 0;
		//milliseconds
		} else if (len == 13) {
            #if INTPTR_MAX == INT64_MAX
			if (isInt(datestr)) {
				long long  milliseconds = strtoll(datestr, NULL, 10);
				dt = milliseconds * 1000;
			}
            #endif
		//seconds
		} else if (len == 10) {
			if (isInt(datestr)) {
				dt = strtoll(datestr, NULL, 10) * 1000000;
			}
		} else if (len == 8) {
			// "20060102"
			setYear(p, 0, 4);
			setMonth(p, 4, 2);
			setDay(p, 6, 2);
			return 0;
		} else if (len == 4) {
			setYear(p, 0, 4);
			return 0;
		} else if (len < 4) {
			return -1;
		}
		if (dt){
			p->t = dt;
			return 0;
		}
		break;

	case dateYearDash:
		// 2006-01
		return 0;

	case dateYearDashDash:
		// 2006-01-02
		// 2006-1-02
		// 2006-1-2
		// 2006-01-2
		return 0;

	case dateYearDashAlphaDash:
		// 2013-Feb-03
		// 2013-Feb-3
		p->daylen = i - p->dayi;
		foundDay(p);
		return 0;

	case dateYearDashDashWs:
		// 2013-04-01
		return 0;

	case dateYearDashDashT:
		return 0;

	case dateDigitDashAlphaDash:
		// 13-Feb-03   ambiguous
		// 28-Feb-03   ambiguous
		// 29-Jun-2016
		length = p->len - (p->moi + p->molen + 1);
		if (length == 4) {
			p->yearlen = 4;
			setYear(p, p->yeari, 4);
			// We now also know that part1 was the day
			p->dayi = 0;
			p->daylen = p->part1Len;
			foundDay(p);
		} else if (length == 2) {
			// We have no idea if this is
			// yy-mon-dd   OR  dd-mon-yy
			//
			// We are going to ASSUME (bad, bad) that it is dd-mon-yy  which is a horible assumption
			p->ambiguousMD = 1;
			p->yearlen = 2;
			setYear(p, p->yeari, 2);
			// We now also know that part1 was the day
			p->dayi = 0;
			p->daylen = p->part1Len;
			foundDay(p);
		}
		return 0;

	case dateDigitDot:
		// 2014.05
		p->molen = i - p->moi;
		foundMonth(p);
		return 0;

	case dateDigitDotDot:
		// 03.31.1981
		// 3.31.2014
		// 3.2.1981
		// 3.2.81
		// 08.21.71
		// 2018.09.30
		return 0;

	case dateDigitWsMoYear:
		// 2 Jan 2018
		// 2 Jan 18
		// 2 Jan 2018 23:59
		// 02 Jan 2018 23:59
		// 12 Feb 2006, 19:17
		return 0;

	case dateDigitWsMolong:
		// 18 January 2018
		// 8 January 2018
		if (p->daylen == 2) {
			setDay(p, 0, 2);
			p->molen = setMonth(p, 3, 0);
			setYear(p, 4+p->molen, 4);
			return 0;
		}
		setDay(p, 0, 2);
		p->molen = setMonth(p, 2, 0);
		setYear(p, 3+p->molen, 4);
		return 0; // parse("2 January 2006", datestr, loc)

	case dateAlphaWsMonth:
		p->yearlen = i - p->yeari;
		foundYear(p);
		return 0;

	case dateAlphaWsMonthMore:
		return 0;

	case dateAlphaWsDigitMoreWs:
		// oct 1, 1970
		p->yearlen = i - p->yeari;
		foundYear(p);
		return 0;

	case dateAlphaWsDigitMoreWsYear:
		// May 8, 2009 5:57:51 PM
		// Jun 7, 2005, 05:57:51
		return 0;

	case dateAlphaWsAlpha:
		return 0;

	case dateAlphaWsAlphaYearmaybe:
		return 0;

	case dateDigitSlash:
		// 3/1/2014
		// 10/13/2014
		// 01/02/2006
		// 2014/10/13
		return 0;

	//case dateDigitChineseYear:
		// dateDigitChineseYear
		//   2014年04月08日
		//strcpy(p->format, "2006年01月02日");
		//return 0;

	//case dateDigitChineseYearWs:
		//p->format = []byte("2006年01月02日 15:04:05")
		//return p, nil

	case dateWeekdayComma:
		// Monday, 02 Jan 2006 15:04:05 -0700
		// Monday, 02 Jan 2006 15:04:05 +0100
		// Monday, 02-Jan-06 15:04:05 MST
		return 0;

	case dateWeekdayAbbrevComma:
		// Mon, 02-Jan-06 15:04:05 MST
		// Mon, 02 Jan 2006 15:04:05 MST
		return 0;

	}


	return -1;
}

static void printall(struct parser* p){
	puts("");
	printf("                                  ");
	if (p->yearbuf[0])
		printf("year: %s ", p->yearbuf);
	if (p->mobuf[0])
		printf("mo: %s ", p->mobuf);
	if (p->daybuf[0])
		printf("day: %s ", p->daybuf);
	if (p->wdaybuf[0])
		printf("wday: %s ", p->wdaybuf);
	if (p->hourbuf[0])
		printf("h: %s ", p->hourbuf);
	if (p->minbuf[0])
		printf("mn: %s ", p->minbuf);
	if (p->secbuf[0])
		printf("s: %s ", p->secbuf);
	if (p->usbuf[0])
		printf("ms: %s ", p->usbuf);
	if (p->ampmbuf[0])
		printf("am: %s ", p->ampmbuf);
	if (p->offsetbuf[0])
		printf("os: %s ", p->offsetbuf);
	if (p->tzbuf[0])
		printf("z: %s ", p->tzbuf);
	puts("");
}

static int monthNum(char* m){
	switch (tolower(m[0])){
	case 'a':
		switch (tolower(m[1])){
		case 'p': return 3;
		case 'u': return 7;
		}
		return -1;
	case 'm':
		switch (tolower(m[2])) {
		case 'r': return 2;
		case 'y': return 4;
		}
		return -1;
	case 'j':
		switch (tolower(m[1])+tolower(m[2])){
		case 207: return 0;
		case 227: return 5;
		case 225: return 6;
		}
		return -1;
	case 'f': return 1;
	case 's': return 8;
	case 'o': return 9;
	case 'n': return 10;
	case 'd': return 11;
	}
	return -1;
}

static int parser2tm(struct parser* p, struct tm* t, date_t* us, int *offset) {
	int a;
	memset(t, 0 , sizeof(struct tm));
	t->tm_isdst = -1;

	if (p->yearbuf[0]){
		a = atoi(p->yearbuf);
		if (a > 999)
			t->tm_year = a - 1900;
		else {
			int now = time(0);
			if (a > now/(60*60*24*367)-30) // 20th century
				t->tm_year = a;
			else //21st century
				t->tm_year = a+100;
		}
	}

	if (p->mobuf[0]){
		if (isalpha(p->mobuf[0])){
			t->tm_mon = monthNum(p->mobuf);
		} else {
			t->tm_mon = atoi(p->mobuf) - 1;
		}
	}

	if (p->daybuf[0]){
		t->tm_mday = atoi(p->daybuf);
	} else {
		t->tm_mday = 1;
	}

	if (p->hourbuf[0]){
		t->tm_hour = atoi(p->hourbuf);
		if (p->ampmbuf[0]) {
			if (tolower(p->ampmbuf[0]) == 'p'){
				t->tm_hour = t->tm_hour % 12 + 12;
			} else if (t->tm_hour == 12 && tolower(p->ampmbuf[0]) == 'a') {
				t->tm_hour = 0;
			}
		}
	}

	if (p->minbuf[0]){
		t->tm_min = atoi(p->minbuf);
	}

	if (p->secbuf[0]){
		t->tm_sec = atoi(p->secbuf);
	}

	if (p->usbuf[0] && us){
		*us = 0;
		int i, multiple = 100000;
		for (i=0; p->usbuf[i] && i<6; i++){
			*us += (p->usbuf[i]-48) * multiple;
			multiple /= 10;
		}
		//printf("decimal: num.%d  from %s\n", *us, p->usbuf);
	}

	if (p->offsetbuf[0] && offset){
		char c = p->offsetbuf[0];
		int sign;
		int minuteOffset = 0;
		if (c == '-') sign = -1;
		else sign = 1;
		int i,j=0;
		for (i=1; i<7 && p->offsetbuf[i]; i++){
			c = p->offsetbuf[i];
			if (isdigit(c)){
				switch (i-j){
				case 1: minuteOffset = 600*(c-48); break;
				case 2: minuteOffset += 60*(c-48); break;
				case 3: minuteOffset += 10*(c-48); break;
				case 4: minuteOffset += c-48;      break;
				}
			} else if (c == ':') {
				j = 1;
			} else {
				return -1;
			}
		}
		*offset = minuteOffset*sign;
	} else if (offset) *offset = 0;
	//printf("\ntm:\n\tsec: %d\n\tmin: %d\n\thour: %d\n\tmday: %d\n\tmon: %d\n\tyear: %d\n\twday: %d\n\tyday: %d\n\tdst: %d\n\n",
			//t->tm_sec, t->tm_min, t->tm_hour, t->tm_mday, t->tm_mon, t->tm_year, t->tm_wday, t->tm_yday, t->tm_isdst);
	//also need to calculate offset for timezones
	//many timezones still missing from buffers

	return 0;
}

static int parse(struct parser* p, date_t* dt, int *offset){
	if (p->t){
		*dt = p->t;
		return 0;
	}
	if (p->fullMonth[0])
		setFullMonth(p, p->fullMonth);
	if (p->skip > 0) {
		p->datestr = p->datestr+p->skip;
	}
	//printall(p);
	struct tm t;
	if (parser2tm(p, &t, dt, offset))
		return -1;
	*dt += mktimegm(&t);
	return 0;
}

int dateparse(const char* datestr, date_t* t, int *offset, int stringlen){
	struct parser p;
	*t = 0;
	if (!stringlen)
		stringlen = strlen(datestr);
	if (parseTime(datestr, &p, stringlen))
		return -1;
	return parse(&p, t, offset);
}

char* datestring(date_t t){
	static char dateprintbuf[30];
	struct tm* tminfo = gmtime64(t);
	strftime(dateprintbuf, 30, "%Y-%m-%d %H:%M:%S", tminfo);
	return dateprintbuf;
}
char* datestringfmt(date_t t, const char* format){
	static char dateprintbuf[30];
	struct tm* tminfo = gmtime64(t);
	strftime(dateprintbuf, 30, format, tminfo);
	return dateprintbuf;
}

date_t nowlocal(){
	time_t sec = time(0);
	struct tm t;
	localtime_r(&sec, &t);
	return mktimegm(&t);
}
date_t nowgm(){
	return (date_t)time(0)*1000000;
}

/*
musl as a whole is licensed under the following standard MIT license:

----------------------------------------------------------------------
Copyright © 2005-2020 Rich Felker, et al.

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
----------------------------------------------------------------------
*/
#include <time.h>

int __days_in_month(int, int);
int __month_to_secs(int, int);
long long __year_to_secs(long long, int *);
long long __tm_to_secs(const struct tm *);
const char *__tm_to_tzname(const struct tm *);
int __secs_to_tm(long long, struct tm *);
void __secs_to_zone(long long, int, int *, long *, long *, const char **);
const char *__strftime_fmt_1(char (*)[100], size_t *, int, const struct tm *, locale_t, int);
extern const char __utc[];


long long __tm_to_secs(const struct tm *tm)
{
	int is_leap;
	long long year = tm->tm_year;
	int month = tm->tm_mon;
	if (month >= 12 || month < 0) {
		int adj = month / 12;
		month %= 12;
		if (month < 0) {
			adj--;
			month += 12;
		}
		year += adj;
	}
	long long t = __year_to_secs(year, &is_leap);
	t += __month_to_secs(month, is_leap);
	t += 86400LL * (tm->tm_mday-1);
	t += 3600LL * tm->tm_hour;
	t += 60LL * tm->tm_min;
	t += tm->tm_sec;
	return t;
}

long long __year_to_secs(long long year, int *is_leap)
{
	if (year-2ULL <= 136) {
		int y = year;
		int leaps = (y-68)>>2;
		if (!((y-68)&3)) {
			leaps--;
			if (is_leap) *is_leap = 1;
		} else if (is_leap) *is_leap = 0;
		return 31536000*(y-70) + 86400*leaps;
	}

	int cycles, centuries, leaps, rem;

	if (!is_leap) is_leap = &(int){0};
	cycles = (year-100) / 400;
	rem = (year-100) % 400;
	if (rem < 0) {
		cycles--;
		rem += 400;
	}
	if (!rem) {
		*is_leap = 1;
		centuries = 0;
		leaps = 0;
	} else {
		if (rem >= 200) {
			if (rem >= 300) centuries = 3, rem -= 300;
			else centuries = 2, rem -= 200;
		} else {
			if (rem >= 100) centuries = 1, rem -= 100;
			else centuries = 0;
		}
		if (!rem) {
			*is_leap = 0;
			leaps = 0;
		} else {
			leaps = rem / 4U;
			rem %= 4U;
			*is_leap = !rem;
		}
	}

	leaps += 97*cycles + 24*centuries - *is_leap;

	return (year-100) * 31536000LL + leaps * 86400LL + 946684800 + 86400;
}


int __month_to_secs(int month, int is_leap)
{
	static const int secs_through_month[] = {
		0, 31*86400, 59*86400, 90*86400,
		120*86400, 151*86400, 181*86400, 212*86400,
		243*86400, 273*86400, 304*86400, 334*86400 };
	int t = secs_through_month[month];
	if (is_leap && month >= 2) t+=86400;
	return t;
}
