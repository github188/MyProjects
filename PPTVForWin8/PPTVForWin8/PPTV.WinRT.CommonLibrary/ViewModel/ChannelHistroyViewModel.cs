﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Collections.ObjectModel;

namespace PPTV.WinRT.CommonLibrary.ViewModel
{
    public class ChannelHistroyViewModel<T>
    {
        public ObservableCollection<T> Items { get; set; }
    }
}
