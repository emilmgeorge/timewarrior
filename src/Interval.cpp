////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2016 - 2023, Thomas Lauf, Paul Beckingham, Federico Hernandez.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// https://www.opensource.org/licenses/mit-license.php
//
////////////////////////////////////////////////////////////////////////////////

#include <Interval.h>
#include <JSON.h>
#include <Lexer.h>
#include <algorithm>
#include <sstream>
#include <timew.h>

////////////////////////////////////////////////////////////////////////////////
bool Interval::operator== (const Interval& other) const
{
  if ((_annotations == other._annotations) &&
      (_tags == other._tags) &&
      (synthetic == other.synthetic) &&
      (id == other.id))
  {
    return Range::operator== (other);
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Interval::operator!= (const Interval& other) const
{
  return ! operator== (other);
}

////////////////////////////////////////////////////////////////////////////////
bool Interval::empty () const
{
  return start.toEpoch () == 0 &&
         end.toEpoch ()   == 0 &&
         _tags.empty () &&
         _annotations.empty ();
}

////////////////////////////////////////////////////////////////////////////////
bool Interval::hasTag (const std::string& tag) const
{
  return std::find (_tags.begin (), _tags.end (), tag) != _tags.end ();
}

////////////////////////////////////////////////////////////////////////////////
const std::set <std::string>& Interval::tags () const
{
  return _tags;
}

////////////////////////////////////////////////////////////////////////////////
void Interval::tag (const std::string& tag)
{
  _tags.insert (tag);
}

////////////////////////////////////////////////////////////////////////////////
void Interval::tag (const std::set <std::string>& tags)
{
  _tags.insert (tags.begin (), tags.end ());
}

////////////////////////////////////////////////////////////////////////////////
void Interval::untag (const std::string& tag)
{
  _tags.erase (tag);
}

////////////////////////////////////////////////////////////////////////////////
void Interval::untag (const std::set <std::string>& tags)
{
  std::set <std::string> updated;

  std::set_difference (
    _tags.begin (), _tags.end (),
    tags.begin (), tags.end (),
    std::inserter (updated, updated.end ())
  );

  _tags = updated;
}

////////////////////////////////////////////////////////////////////////////////
void Interval::clearTags ()
{
  _tags.clear ();
}

////////////////////////////////////////////////////////////////////////////////
std::string Interval::serialize () const
{
  std::stringstream out;
  out << "inc";

  if (start.toEpoch ())
    out << " " << start.toISO ();

  if (end.toEpoch ())
    out << " - " << end.toISO ();

  if (! _tags.empty ())
  {
    out << " #";
    for (auto& tag : _tags)
      out << ' ' << quoteIfNeeded (tag);
  }

  if (! _annotations.empty ())
  {
    out << (_tags.empty () ? " #" : "");
    auto it = _annotations.begin();
    for (; it != _annotations.end(); it++)
    {
      Datetime t = it->first;
      std::string a = it->second;
      out << " # " << t.toISO() << " - \"" << escape (a, '"') << "\"";
    }
  }

  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
std::string Interval::json () const
{
  std::stringstream out;
  out << "{";

  if (! empty ())
  {
    out << "\"id\":" << id;

    if (is_started ())
    {
      out << ",\"start\":\"" << start.toISO () << "\"";
    }

    if (is_ended ())
    {
      out << ",\"end\":\"" << end.toISO () << "\"";
    }

    if (! _tags.empty ())
    {
      std::string tags;
      for (auto& tag : _tags)
      {
        if (tags[0])
          tags += ',';

        tags += "\"" + json::encode (tag) + "\"";
      }

      out << ",\"tags\":["
          << tags
          << ']';
    }

    if (! _annotations.empty ())
    {
      out << ",\"annotations\": {";
      auto it = _annotations.begin();
      for (; it != _annotations.end(); it++)
      {
        Datetime t = it->first;
        std::string a = it->second;
        if(it != _annotations.begin())
          out << ", ";
        out << "\"" << t.toISO() << "\": \"" << json::encode(a) << "\"";
      }
      out << "}";
    }
  }
  out << "}";

  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
std::string Interval::dump () const
{
  std::stringstream out;

  out << "interval";

  if (id)
    out << " @" << id;

  if (start.toEpoch ())
    out << " " << start.toISOLocalExtended ();

  if (end.toEpoch ())
    out << " - " << end.toISOLocalExtended ();

  if (! _tags.empty ())
  {
    out << " #";
    for (auto& tag : _tags)
      out << ' ' << quoteIfNeeded (tag);
  }

  if (synthetic)
    out << " synthetic";

  return out.str ();
}

void Interval::setRange (const Range& range)
{
  setRange (range.start, range.end);
}

void Interval::setRange (const Datetime& start, const Datetime& end)
{
  this->start = start;
  this->end = end;
}

////////////////////////////////////////////////////////////////////////////////
void Interval::addAnnotation(const Datetime& time, const std::string& annotation)
{
  this->_annotations.emplace(time, annotation);
}

////////////////////////////////////////////////////////////////////////////////
void Interval::setAnnotation(const Datetime& time, const std::string& annotation)
{
  auto it = this->_annotations.find(time);
  if(it != this->_annotations.end())
    this->_annotations[time] = annotation;
}

////////////////////////////////////////////////////////////////////////////////
void Interval::removeAnnotation(const Datetime& time)
{
  auto key = _annotations.find(time);
  if(key != _annotations.end())
    _annotations.erase(key);
}

////////////////////////////////////////////////////////////////////////////////
std::string Interval::getAnnotation(const Datetime& time) const
{
  auto ret = _annotations.find(time);
  if(ret != _annotations.end())
    return (*ret).second;
  return std::string();
}

////////////////////////////////////////////////////////////////////////////////
std::map <Datetime,  std::string> Interval::getAnnotations() const
{
  return _annotations;
}

////////////////////////////////////////////////////////////////////////////////
