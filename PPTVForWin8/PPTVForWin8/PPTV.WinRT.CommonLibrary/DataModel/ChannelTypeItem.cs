﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace PPTV.WinRT.CommonLibrary.DataModel
{
    public class ChannelTypeItem : BindableBase
    {
        public int TypeId { get; set; }

        private string _typeName;
        public string TypeName
        {
            get { return _typeName; }
            set { this.SetProperty(ref _typeName, value); }
        }

        private string _imageUri;
        public string ImageUri
        {
            get { return _imageUri; }
            set { this.SetProperty(ref _imageUri, value); }
        }
    }
}
