# Third-party notices

## JUCE

This project fetches JUCE 9.0.3 from the official JUCE repository at configure
time. JUCE is offered under the GNU Affero General Public License v3 or a
commercial license. The JUCE license text and the terms that apply to your
use must be reviewed before distributing the built plugin. See the versioned
[JUCE 9.0.3 license](https://github.com/juce-framework/JUCE/blob/9.0.3/LICENSE.md)
and the [JUCE CMake documentation](https://github.com/juce-framework/JUCE/blob/9.0.3/docs/CMake%20API.md).

The project source is offered under GNU AGPL version 3 or later so the
community-license build can be shared with its corresponding source. The full
license is at <https://www.gnu.org/licenses/agpl-3.0.en.html>. This is suitable
for sharing source and builds with friends; do not use it for a closed-source
commercial release without obtaining the appropriate JUCE commercial license
and reviewing the terms.

## VST3

The plugin is built as a VST3 through JUCE. It does not include Steinberg's
SDK as a vendored copy; JUCE manages the VST3 implementation during the build.
