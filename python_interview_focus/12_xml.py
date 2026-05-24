#!/usr/bin/python3

# 这个文件里不是一个单独的普通函数，而是一个 SAX 解析器处理类 MovieHandler。它的作用是：在解析 movies.xml 时，遇到不同 XML 节点就由对应回调方法来处理。
# 核心流程是这样的：
# startElement(tag, attributes)

# 当解析器遇到一个元素开始标签时会调用它。
# 它把当前正在处理的标签名记到 self.CurrentData。
# 如果标签是 movie，就打印电影标题 title 属性。
# characters(content)

# 当解析器读到元素内部的文本内容时会调用它。
# 根据当前标签名，把文本保存到对应字段里，比如 type、format、year 等。
# endElement(tag)

# 当解析器遇到元素结束标签时会调用它。
# 它根据 self.CurrentData 判断刚刚结束的是哪个字段，然后把保存的内容打印出来。
# 最后把 self.CurrentData 清空。
# 你可以把它理解成：

# startElement 负责“开始一个标签”
# characters 负责“读取标签里的文本”
# endElement 负责“标签结束后输出结果

import xml.sax

class MovieHandler( xml.sax.ContentHandler ):
   def __init__(self):
      self.CurrentData = ""
      self.type = ""
      self.format = ""
      self.year = ""
      self.rating = ""
      self.stars = ""
      self.description = ""

   # 元素开始调用
   def startElement(self, tag, attributes):
      self.CurrentData = tag
      if tag == "movie":
         print ("*****Movie*****")
         title = attributes["title"]
         print ("Title:", title)

   # 元素结束调用
   def endElement(self, tag):
      if self.CurrentData == "type":
         print ("Type:", self.type)
      elif self.CurrentData == "format":
         print ("Format:", self.format)
      elif self.CurrentData == "year":
         print ("Year:", self.year)
      elif self.CurrentData == "rating":
         print ("Rating:", self.rating)
      elif self.CurrentData == "stars":
         print ("Stars:", self.stars)
      elif self.CurrentData == "description":
         print ("Description:", self.description)
      self.CurrentData = ""

   # 读取字符时调用
   def characters(self, content):
      if self.CurrentData == "type":
         self.type = content
      elif self.CurrentData == "format":
         self.format = content
      elif self.CurrentData == "year":
         self.year = content
      elif self.CurrentData == "rating":
         self.rating = content
      elif self.CurrentData == "stars":
         self.stars = content
      elif self.CurrentData == "description":
         self.description = content

if ( __name__ == "__main__"):

   # 创建一个 XMLReader
   parser = xml.sax.make_parser()
   # 关闭命名空间
   parser.setFeature(xml.sax.handler.feature_namespaces, 0)

   # 重写 ContextHandler
   Handler = MovieHandler()
   parser.setContentHandler( Handler )

   parser.parse("movies.xml")