# -*- coding: utf-8 -*-
"""
calendar_sync.py - 日程同步工具
小智·智家 XiaoZhi SmartHome

通过 CalDAV 与 NAS 日历同步，支持：
- 拉取未来 N 天日程（供小智播报）
- 创建待办/日程（语音口述记录）
"""
import argparse
import logging
from datetime import datetime, timedelta

log = logging.getLogger("calendar_sync")


def pull_events(caldav_url, user, password, days=7):
    """通过 CalDAV REPORT 查询未来 N 天日程（演示实现）"""
    try:
        import caldav
    except ImportError:
        log.error("caldav not installed: pip install caldav")
        return []

    client = caldav.DAVClient(url=caldav_url, username=user, password=password)
    principal = client.principal()
    events = []
    start = datetime.now()
    end = start + timedelta(days=days)
    for cal in principal.calendars():
        results = cal.date_search(start=start, end=end)
        for r in results:
            events.append({"calendar": cal.name, "data": r.data})
    return events


def create_event(caldav_url, user, password, summary, dtstart, dtend=None):
    """创建日程事件"""
    try:
        import caldav
    except ImportError:
        log.error("caldav not installed: pip install caldav")
        return False

    client = caldav.DAVClient(url=caldav_url, username=user, password=password)
    principal = client.principal()
    cal = principal.calendars()[0]
    end = dtend or (dtstart + timedelta(hours=1))
    cal.save_event(
        dtstart=dtstart,
        dtend=end,
        summary=summary,
    )
    log.info("event created: %s @ %s", summary, dtstart)
    return True


def main():
    logging.basicConfig(level=logging.INFO)
    parser = argparse.ArgumentParser(description="XiaoZhi SmartHome Calendar Sync")
    parser.add_argument("--caldav-url", required=True)
    parser.add_argument("--user", default="")
    parser.add_argument("--password", default="")
    parser.add_argument("--action", choices=["pull", "create"], default="pull")
    parser.add_argument("--days", type=int, default=7)
    parser.add_argument("--summary", default="")
    parser.add_argument("--dtstart", default=None)
    args = parser.parse_args()

    if args.action == "pull":
        events = pull_events(args.caldav_url, args.user, args.password, args.days)
        for e in events:
            print(f"[{e['calendar']}] {e['data'][:200]}")
        print(f"total: {len(events)}")
    elif args.action == "create":
        start = datetime.fromisoformat(args.dtstart) if args.dtstart else datetime.now()
        create_event(args.caldav_url, args.user, args.password, args.summary, start)


if __name__ == "__main__":
    main()
