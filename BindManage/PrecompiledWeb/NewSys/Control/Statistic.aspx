﻿<%@ page language="C#" masterpagefile="~/Control/ProductMaster.master" autoeventwireup="true" inherits="Control_Default, App_Web_statistic.aspx.cc961b29" title="统计信息管理" %>
<asp:Content ID="Content1" ContentPlaceHolderID="ContentPlaceHolder1" Runat="Server">
    <script type="text/javascript" src="Js/Statistic.js"></script>
    <asp:ScriptManager ID="statisticsm" runat="server" >
        <Services>
            <asp:ServiceReference Path="StatisticManage.asmx" />
        </Services>
    </asp:ScriptManager>
    <div id="statisticmanage_grid_div" style="width:100%; height:100%; position:absolute;"></div>
</asp:Content>

