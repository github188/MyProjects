﻿<%@ WebHandler Language="C#" Class="list" %>

using System;
using System.Web;

using Synacast.BoxManage.List.Lists;
using Synacast.BoxManage.Client;
using Synacast.BoxManage.Client.Help;

public class list : IHttpHandler {
    
    public void ProcessRequest (HttpContext context) {
        var filter = new ListFilter();
        filter.c = ClientUtils.FormatIntV(context.Request["c"], 200, "c");
        filter.s = ClientUtils.FormatIntV(context.Request["s"], "s");
        filter.conlen = ClientUtils.FormatInt(context.Request["conlen"]);
        filter.auth = context.Request["auth"];
        filter.flag = context.Request["flag"];
        filter.platform = context.Request["platform"];
        filter.rankingspan = ClientUtils.FormatRankingSpan(context.Request["rankingspan"]);
        filter.beginletter = ClientUtils.FormatBeginLetter(context.Request["beginletter"], "beginletter");
        filter.lang = ClientUtils.FormatLanguage(context.Request["lang"]);
        filter.ver = ClientUtils.FormatDefaultInt(context.Request["ver"], 1);
        if (filter.ver <= 1)
        {
            filter.tag = ClientUtils.FormatSimpleChinese(context.Request["tag"]);
        }
        else
        {
            filter.ntags = ClientUtils.FormatSimpleChinese(context.Request["ntags"]);
            filter.treeleftid = ClientUtils.FormatInt(context.Request["treeleftid"]);
            filter.screen = context.Request["screen"];
        }
        filter.order = ClientUtils.FormatOrder(context.Request["order"]);
        filter.type = ClientUtils.FormatListType(context.Request["type"], filter.tag, filter.ntags, filter.treeleftid);
        var list = ServiceInvoke<IList>.CreateContract("boxlist");
        context.Response.Write(ServiceInvoke<IList>.Invoke<string>(list, proxy => proxy.VideoList(filter)));
    }
 
    public bool IsReusable {
        get {
            return false;
        }
    }

}