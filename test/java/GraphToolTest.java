import static org.junit.jupiter.api.Assertions.*;
import org.junit.jupiter.api.BeforeAll;
import org.junit.jupiter.api.Test;
import java.io.IOException;
import java.util.List;

public class GraphToolTest {
  static GraphTool.Graph graph;

  @BeforeAll
  static void setup() throws IOException {
    graph = GraphTool.buildGraph("src/Easy Test.txt");
  }

  @Test
  void testBridgeWordExists() {
    List<String> result = graph.findBridgeWords("more", "wrote");
    assertTrue(result.contains("data"), "桥接词应包含 data");
  }

  @Test
  void testBridgeWordExists2() {
    List<String> result = graph.findBridgeWords("detailed", "with");
    assertTrue(result.contains("report"), "桥接词应包含 report");
  }

  @Test
  void testNoBridgeWord() {
    List<String> result = graph.findBridgeWords("wrote", "data");
    assertTrue(result.isEmpty(), "桥接词应该不存在");
  }

  @Test
  void testWord1NotInGraph() {
    List<String> result = graph.findBridgeWords("abc", "data");
    assertTrue(result.isEmpty(), "word1 不存在，应返回空");
  }

  @Test
  void testWord2NotInGraph() {
    List<String> result = graph.findBridgeWords("scientist", "xyz");
    assertTrue(result.isEmpty(), "word2 不存在，应返回空");
  }

  @Test
  void testBothWordsNotInGraph() {
    List<String> result = graph.findBridgeWords("abc", "xyz");
    assertTrue(result.isEmpty(), "两个词都不存在，应返回空");
  }

  @Test
  void testSameWord() {
    List<String> result = graph.findBridgeWords("wrote", "wrote");
    assertTrue(result.isEmpty(), "word1=word2，应返回空");
  }
}

