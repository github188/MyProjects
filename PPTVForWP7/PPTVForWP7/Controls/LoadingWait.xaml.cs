﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Shapes;

namespace PPTVForWP7.Controls
{
    public partial class LoadingWait : UserControl
    {
        public LoadingWait()
        {
            InitializeComponent();
        }

        public string Text
        {
            get
            {
                return WaitText.Text;
            }
            set
            {
                WaitText.Text = value;
            }
        }

        public Brush TextForeground
        {
            get
            {
                return WaitText.Foreground;
            }
            set
            {
                WaitText.Foreground = value;
            }
        }
    }
}
